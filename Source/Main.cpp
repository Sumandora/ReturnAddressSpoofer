#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

void __attribute((naked)) ReturnGadget()
{
	__asm volatile("ret;");
}
void* RetAddrSpoofer::leaveRet = reinterpret_cast<void*>(ReturnGadget);

const int argumentLength = 1000 * 1000 * 8; // approximate the maximum that the stack can handle

size_t TestFunction(const char str[argumentLength])
{
	assert(__builtin_extract_return_addr(__builtin_return_address(0)) == RetAddrSpoofer::leaveRet);
	return strlen(str);
}

int main()
{
	char str[argumentLength + 1];
	for (char& i : str)
		i = 'A';
	str[argumentLength] = '\0';

	int length = RetAddrSpoofer::Invoke<int>(reinterpret_cast<std::uintptr_t>(str), 0, 0, 0, 0, 0, reinterpret_cast<void*>(TestFunction));

	assert(length == argumentLength);

	printf("String length was: %d\n", length);
}
