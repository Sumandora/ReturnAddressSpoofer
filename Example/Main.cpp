#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

void __attribute((naked)) returnGadget()
{
	__asm volatile("leave; ret;");
}
void* RetAddrSpoofer::leaveRet = reinterpret_cast<void*>(returnGadget);

size_t testFunction(const char* str, size_t& magicNumber)
{
	magicNumber = 1337;
	assert(__builtin_extract_return_addr(__builtin_return_address(0)) == RetAddrSpoofer::leaveRet);
	return strlen(str);
}

int main()
{
	const char* str = "Hello, world!";

	size_t magicNumber = 0;
	auto length = RetAddrSpoofer::invoke<size_t, const char*, size_t&>(reinterpret_cast<void*>(testFunction), str, magicNumber);

	assert(magicNumber == 1337);
	assert(length == strlen(str));

	printf("String length was: %ld\n", length);
}
