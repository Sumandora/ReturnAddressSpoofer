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

	inline void __attribute((naked)) _Invoke()
	{
		__asm("pop %rax;" // ret addr
			  "pop %r10;" // method addr
		);
		__asm("push %0"
			  :
			  : "m"(leaveRet));
		__asm("jmp *%r10;");
	}

#pragma GCC push_options
#pragma GCC optimize("no-optimize-sibling-calls")
#pragma GCC optimize("no-omit-frame-pointer")
	template <
		typename Ret,
		typename RDI = std::uintptr_t,
		typename RSI = std::uintptr_t,
		typename RDX = std::uintptr_t,
		typename RCX = std::uintptr_t,
		typename R8 = std::uintptr_t,
		typename R9 = std::uintptr_t,
		typename... StackArgs>
	inline Ret __attribute((noinline, disable_tail_calls, force_align_arg_pointer)) Invoke(RDI rdi, RSI rsi, RDX rdx, RCX rcx, R8 r8, R9 r9, void* functionAddress, StackArgs... stackArgs)
	{
		return reinterpret_cast<Ret (*)(RDI, RSI, RDX, RCX, R8, R9, void*, StackArgs...)>(_Invoke)(rdi, rsi, rdx, rcx, r8, r9, functionAddress, stackArgs...);
	}
#pragma GCC pop_options

}
#endif
