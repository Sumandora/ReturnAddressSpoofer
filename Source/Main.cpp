#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

void __attribute((naked)) ReturnGadget()
{
	__asm volatile("leave; ret;");
}
void* RetAddrSpoofer::leaveRet = reinterpret_cast<void*>(ReturnGadget);

const int argumentLength = 1000 * 1000 * 8; // approximate the maximum that the stack can handle

size_t TestFunction(const char str[argumentLength], long rsi, long rdx, long rcx, float xmm0, long r8, float xmm1, long r9, long stackArg1, float xmm2)
{
	assert(rsi == rdx == rcx == xmm0 == r8 == xmm1 == r9 == stackArg1 == xmm2 == 1);
	assert(__builtin_extract_return_addr(__builtin_return_address(0)) == RetAddrSpoofer::leaveRet);
	return strlen(str);
}

int main()
{
	char str[argumentLength + 1];
	for (char& i : str)
		i = 'A';
	str[argumentLength] = '\0';

	size_t length = RetAddrSpoofer::Invoke<size_t>(reinterpret_cast<std::uintptr_t>(str), 1l, 1l, 1l, 1l, 1l, reinterpret_cast<void*>(TestFunction), 1l, 1.0f, 1.0f, 1.0f /* Floats are passed using XMM registers, their position doesn't matter, but their order does! */);

	assert(length == argumentLength);

	printf("String length was: %ld\n", length);
}
