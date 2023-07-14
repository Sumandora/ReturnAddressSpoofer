#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

#include <cstdint>

namespace RetAddrSpoofer {

	/*
	 * Explanation for leaveRet:
	 * The return address spoofer expects this to be set
	 * This has to be a byte-sequence which contains the following:
	 * c9	leave
	 * c3	ret
	 */
	extern void* leaveRet;

	template <typename Ret>
	inline Ret __attribute((naked)) Invoke(std::uintptr_t rdi, std::uintptr_t rsi, std::uintptr_t rdx, std::uintptr_t rcx, std::uintptr_t r8, std::uintptr_t r9, void* functionAddress)
	{
		__asm("pop %rax;"
			  "pop %r10;"
			  "push %rax;");
		__asm("push %0"
			  :
			  : "m"(leaveRet));
		__asm("jmp *%r10;");
	}

}
#endif
