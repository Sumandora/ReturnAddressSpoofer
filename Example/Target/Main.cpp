#include <cassert>
#include <cstring>

void __attribute((naked
#ifndef __clang__
	, optimize("Ofast") /* stop GCC from inserting any boilerplate like (GOT) */
#endif
	)) returnGadget()
{
	asm volatile("leave; ret;");
}

std::size_t testFunction(const char* str, std::size_t* magicNumber)
{
	*magicNumber = 1337;
	assert(__builtin_extract_return_addr(__builtin_return_address(0)) == returnGadget);
	return strlen(str);
}