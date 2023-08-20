#include "RetAddrSpoofer.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

void __attribute((naked)) returnGadget()
{
	__asm volatile("leave; ret;");
}
void* RetAddrSpoofer::leaveRet = reinterpret_cast<void*>(returnGadget);

const int argumentLength = 1000 * 1000 * 8; // approximate the maximum that the stack can handle

size_t testFunction(const char* str)
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

	int length = RetAddrSpoofer::invoke<int, const char*>(reinterpret_cast<void*>(testFunction), str);

	assert(length == argumentLength);

	printf("String length was: %d\n", length);
}
