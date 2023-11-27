#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

// Defined in target
extern void returnGadget();
std::size_t testFunction(const char* str, std::size_t* magicNumber);


const void* RetAddrSpoofer::leaveRet = reinterpret_cast<void*>(returnGadget);

int main()
{
	const char* str = "Hello, world!";

	size_t magicNumber = 0;
	auto length = RetAddrSpoofer::invoke<size_t, const char*, size_t*>(reinterpret_cast<void*>(testFunction), str, &magicNumber);

	assert(magicNumber == 1337);
	assert(length == strlen(str));

	std::cout << "String length was: " << length << std::endl;
}
