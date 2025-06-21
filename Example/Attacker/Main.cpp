#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <concepts>
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
	auto length = RetAddrSpoofer::invoke(testFunction, str, &magicNumber);

	static_assert(std::same_as<decltype(length), std::size_t>);

	assert(magicNumber == 1337);
	assert(length == strlen(str));

	std::cout << "String length was: " << length << std::endl;
}
