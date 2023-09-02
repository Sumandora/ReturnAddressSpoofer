#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

#include <utility>

namespace RetAddrSpoofer {

	/*
	 * Explanation for leaveRet:
	 * The return address spoofer expects this to be set
	 * This has to be a byte-sequence which contains the following:
	 * c9	leave
	 * c3	ret
	 */
	extern void* leaveRet;

	template <typename Ret, typename... Args> requires std::conjunction_v<std::negation<std::is_reference<Args>>...>
	static Ret __attribute((noinline, optimize("O0"))) invoke(void* method, Args... args)
	{
		reinterpret_cast<Ret (*)(Args...)>(method)(args...);

		__asm("mov %0, %%rax;"
			  :
			  : "m"(leaveRet));
		__asm("push %rax;"
			  "nop;"
			  "nop;"
			  "nop;"
			  "nop;");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
		__asm("");
	}
#pragma GCC diagnostic pop

}
#endif
