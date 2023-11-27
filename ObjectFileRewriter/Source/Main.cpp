#include <any>
#include <cassert>
#include <cstring>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <ranges>
#include <span>
#include <vector>

#include "distorm.h"

namespace fs = std::filesystem;

_DecodeType mode =
#ifdef __x86_64
	Decode64Bits
#else
	Decode32Bits
#endif
;

struct DisassembleResult {
	void* addr;
	_DecodedInst inst;
};

DisassembleResult disassemble(void* addr)
{
	_DecodedInst inst;
	unsigned int used = 0;
	distorm_decode(0, reinterpret_cast<unsigned char*>(addr), 0xFF, mode, &inst, 1, &used);
	return { addr, inst };
}

enum class Result {
	SUCCESSFUL,
	FAILED,
	ALREADY_MUTATED
};

std::vector<DisassembleResult> mapFunction(std::span<std::byte> functionBytes) {
	std::vector<DisassembleResult> mappedFunction;
	std::byte* addr = &functionBytes.front();

	while(addr <= &functionBytes.back()) {
		auto res = disassemble(addr);
		mappedFunction.push_back(res);
		addr += res.inst.size;
	}

	return mappedFunction;
}

bool isAlreadyMutated(const std::vector<DisassembleResult>& mappedFunction) {
	for(auto it = mappedFunction.begin(); it != mappedFunction.end(); it++) {
		auto currIt = it;
		if(std::strcmp(reinterpret_cast<const char*>(currIt->inst.mnemonic.p), "NOP") != 0)
			continue; // Must be NOP

		currIt++;
		if(std::strcmp(reinterpret_cast<const char*>(currIt->inst.mnemonic.p), "MOV") != 0)
			continue; // Must be at least one MOV

		currIt++;
		while (std::strcmp(reinterpret_cast<const char*>(currIt->inst.mnemonic.p), "MOV") == 0)
			currIt++; // Skip all MOVs

		if(std::strcmp(reinterpret_cast<const char*>(currIt->inst.mnemonic.p), "PUSH") != 0)
			continue; // Must be PUSH

		currIt++;
		if(std::strcmp(reinterpret_cast<const char*>(currIt->inst.mnemonic.p), "JMP") != 0)
			continue; // Must be JMP

		return true;
	}
	return false;
};

Result mutateNextCall(std::span<std::byte> functionBytes)
{
	std::vector<DisassembleResult> mappedFunction = mapFunction(functionBytes);

	if(isAlreadyMutated(mappedFunction))
		return Result::ALREADY_MUTATED;

	auto iterateUntil = [](const std::string& name, auto begin, auto end, auto cond) {
		for(auto it = begin; it != end; it++) {
			if (cond(it))
				return it;
		}
		throw std::runtime_error(name + ": Iteration failed");
	};

	bool foundNop = false;
	auto firstNop = iterateUntil("First NOP", mappedFunction.rbegin(), mappedFunction.rend(), [&foundNop](auto it) {
		if (std::strcmp(reinterpret_cast<const char*>((it+1)->inst.mnemonic.p), "NOP") == 0)
			foundNop = true;
		else if (foundNop)
			return true;
		return false;
	});

	auto movInstruction = iterateUntil("MOV instruction", firstNop + 1, mappedFunction.rend(), [](auto it) {
		return std::strcmp(reinterpret_cast<const char*>((it+1)->inst.mnemonic.p), "MOV") != 0;
	});

	auto callInstruction = iterateUntil("CALL instruction", movInstruction, mappedFunction.rend(), [](auto it) {
		return std::strcmp(reinterpret_cast<const char*>(it->inst.mnemonic.p), "CALL") == 0;
	});

	auto firstNopAddr = reinterpret_cast<unsigned char*>(firstNop->addr);
	auto movInstructionAddr = reinterpret_cast<unsigned char*>(movInstruction->addr);
	auto callInstructionAddr = reinterpret_cast<unsigned char*>(callInstruction->addr);

	// Clear everything after the method ended (we are a noreturn, this will remove the ud2)
	auto end = firstNopAddr + callInstruction->inst.size;
	memset(end, 0x90, reinterpret_cast<unsigned char*>(&functionBytes.back()) - end);
	functionBytes.back() = static_cast<std::byte>(0xCC); // int 3

	// Move the call instruction down
	memcpy(firstNop->addr, callInstruction->addr, callInstruction->inst.size);
	memset(callInstruction->addr, 0x90, movInstructionAddr - callInstructionAddr);

	// Convert call instruction to jmp instruction
	*(firstNopAddr + callInstruction->inst.size - 1) += 0x10;

	// Rewrite mov sequence to use counter when accumulator is in use
	if(std::strstr(reinterpret_cast<char*>(callInstruction->inst.operands.p), "AX") &&
		std::strstr(reinterpret_cast<char*>(movInstruction->inst.operands.p), "AX")) {
		// Remember that we are still dealing with reverse iterators, when we subtract from those instructions, we actually go forward
		switch (mode) {
		case Decode64Bits: {
			*(movInstructionAddr + 2) = *(movInstructionAddr + 1) != 0x8b ? 0x0c : 0x0d;
			auto nextInstruction = movInstruction - 1;
			if(std::strcmp(reinterpret_cast<const char*>(nextInstruction->inst.mnemonic.p), "MOV") == 0) {
				*(reinterpret_cast<unsigned char*>(nextInstruction->addr) + 2) = 0x09;
				nextInstruction--;
			}
			*reinterpret_cast<unsigned char*>(nextInstruction->addr) = 0x51;
			break;
		}
		case Decode32Bits: {
			bool clangOnO0 = movInstruction->inst.size == 4; // Don't ask clang is weird
			*(movInstructionAddr + 1) = clangOnO0 ? 0x4C : 0x8B;
			auto nextInstruction = movInstruction - 1;
			if(clangOnO0 && std::strcmp(reinterpret_cast<const char*>(nextInstruction->inst.mnemonic.p), "MOV") == 0) {
				*(reinterpret_cast<unsigned char*>(nextInstruction->addr) + 1) = 0x89;
				nextInstruction--;
			}
			if(std::strcmp(reinterpret_cast<const char*>(nextInstruction->inst.mnemonic.p), "MOV") == 0) {
				*(reinterpret_cast<unsigned char*>(nextInstruction->addr) + 1) = 0x09;
				nextInstruction--;
			}
			*reinterpret_cast<unsigned char*>(nextInstruction->addr) = 0x51;
			break;
		}
		default: break;
		}
	}

	assert(isAlreadyMutated(mapFunction(functionBytes))); // Is function mutated now?

	return Result::SUCCESSFUL;
}

void processObjectFile(const fs::path& file_path)
{
	static const std::string objdumpLine = ".text._ZN14RetAddrSpoofer6invoke";

	std::string path = fs::absolute(file_path).string();
	std::cout << "Processing " << path << std::endl;

	FILE* objdump = popen(("objdump -h " + path).c_str(), "r");

	struct MutableFunction {
		std::string name;
		std::uintptr_t begin;
		std::uintptr_t end;

		MutableFunction(std::string&& name, std::uintptr_t begin, std::uintptr_t end)
			: name(std::move(name))
			, begin(begin)
			, end(end)
		{
		}
	};
	std::vector<MutableFunction> mutableFunctions;

	char* lineptr = nullptr;
	size_t len = 0;
	while (getline(&lineptr, &len, objdump) != -1) {
		std::string line{ lineptr };
		if (line.find(objdumpLine) == std::string::npos) // Find the name of the function
			continue;

		std::vector<std::string> tableLine; // { index, section.name, size, x, x, address, x }

		std::string lineWalker = line;
		while (!lineWalker.empty()) {
			size_t next = lineWalker.find_first_not_of(' ');
			if(next != std::string::npos)
				lineWalker = lineWalker.substr(next);
			size_t length = lineWalker.find(' ');
			if(length == std::string::npos)
				length = lineWalker.length();
			tableLine.push_back(lineWalker.substr(0, length));
			lineWalker = lineWalker.substr(length);
		}

		std::string name = tableLine.at(1);
		std::string address = tableLine.at(5);
		std::string size = tableLine.at(2);

		// Remove prepended 0s
		address = address.substr(address.find_first_not_of('0'));
		size = size.substr(size.find_first_not_of('0'));

		std::uintptr_t parsedAddress = std::stoll(address, nullptr, 16);
		std::uintptr_t parsedSize = std::stoll(size, nullptr, 16);
		mutableFunctions.emplace_back(std::move(name), parsedAddress, parsedAddress + parsedSize);
	}

	if (lineptr)
		free(lineptr);

	pclose(objdump);

	std::fstream fs{ file_path, std::ios::in | std::ios::binary | std::ios::out };
	auto fileSize = fs::file_size(file_path);
	std::byte fileBytes[fileSize];
	fs.read(reinterpret_cast<char*>(fileBytes), static_cast<long>(fileSize) /* this is very stupid, this implies that reading negative lengths is a thing */);

	size_t successful = 0;
	size_t failed = 0;
	size_t skipped = 0;

	for (const MutableFunction& function : mutableFunctions) {
		std::string functionString = std::format("[{:x};{:x}]", function.begin, function.end);
		std::cout << "Mutating the function at " << functionString << std::endl;
		switch (mutateNextCall(std::span<std::byte>{ fileBytes + function.begin, fileBytes + function.end })) {
		case Result::SUCCESSFUL: {
			std::cout << "Successfully mutated the function at " << functionString << std::endl;
			successful++;
			break;
		}
		case Result::FAILED: {
			std::cout << "Failed to mutate the function at " << functionString << std::endl;
			failed++;
			break;
		}
		case Result::ALREADY_MUTATED: {
			std::cout << "Skipped the function at " << functionString << ", because it was already mutated" << std::endl;
			skipped++;
			break;
		}
		}
	}

	std::cout << "Processed " << mutableFunctions.size() << " calls (" << successful << " successful, " << failed << " failed, " << skipped << " skipped)" << std::endl;

	assert(failed == 0); // This shouldn't happen, please tell me if it does.

	fs.seekg(0, std::ios::beg);
	fs.write(reinterpret_cast<char*>(fileBytes), static_cast<long>(fileSize));
	fs.close();
}

void iterateFolder(const std::string& path)
{
	if (fs::is_regular_file(path)) {
		processObjectFile(path);
		return;
	}
	for (const auto& entry : fs::directory_iterator(path)) {
		const fs::path& file_path = entry.path();
		if (fs::is_regular_file(file_path) && file_path.extension() == ".o") {
			processObjectFile(file_path);
		} else if (fs::is_directory(file_path)) {
			iterateFolder(file_path); // Recursively process subdirectories
		}
	}
}

int main(int argc, char** argv)
{
	std::string inputPath;
	int opt;
	while((opt = getopt(argc, argv, "hi:m:")) != -1) {
		switch (opt) {
		case 'h':
		default: {
			std::cout << "Valid options:\n\t-i\tInput file/folder\n\t-m\tMode (amount of bits disassembled in)" << std::endl;
			return 0;
		}
		case 'i': {
			inputPath = optarg;
			continue;
		}
		case 'm': {
			auto bits = std::stoull(optarg, nullptr, 10);
			switch (bits) {
			case 16:
				mode = Decode16Bits;
				break;
			case 32:
				mode = Decode32Bits;
				break;
			case 64:
				mode = Decode64Bits;
				break;
			default:
				std::cerr << "Invalid amount of bits, only valid amounts are [16,32,64]" << std::endl;
				return 1;
			}
			continue;
		}
		}
	}

	if(mode == Decode16Bits) {
		std::cerr << "Warning: 16 bit mode is untested and is unlikely to work" << std::endl;
	}

	if(inputPath.empty()) {
		std::cerr << "Input path is required" << std::endl;
		return 1;
	}

	iterateFolder(inputPath);
	return 0;
}
