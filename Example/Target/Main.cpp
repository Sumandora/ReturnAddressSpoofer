#include <cassert>
#include <cstring>

extern void __attribute((naked
#ifndef __clang__
	,
	optimize("Ofast") /* stop GCC from inserting any boilerplate like (GOT) */
#endif
	))
returnGadget()
{
	// NOLINTBEGIN(hicpp-no-assembler)
#ifndef __x86_64
	asm volatile("leave;");
#endif
	asm volatile("ret;");
	// NOLINTEND(hicpp-no-assembler)
}

extern std::size_t testFunction(const char* str, std::size_t* magicNumber)
{
	*magicNumber = 1337;
	assert(__builtin_extract_return_addr(__builtin_return_address(0)) == returnGadget);
	return strlen(str);
}
