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
	extern const void* leaveRet;


#pragma diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma push_options
#pragma GCC optimize("no-omit-frame-pointer")
	template <typename Ret, typename... Args> requires std::conjunction_v<std::negation<std::is_reference<Args>>...>
	Ret __attribute((noinline, force_align_arg_pointer,
#ifdef __clang__
		optnone
#else
		optimize("O0")
#endif
		)) invoke(void* method, Args... args)
	{
		reinterpret_cast<Ret (*)(Args...)>(method)(args...);

#ifdef __x86_64
#define ACCUMULATOR "rax"
#else
#define ACCUMULATOR "eax"
#endif
		asm volatile("mov %0, %%" ACCUMULATOR ";"
			:
			: "m"(leaveRet));
		asm volatile("push %" ACCUMULATOR ";"
#undef ACCUMULATOR
			"nop;"
			"nop;"
			"nop;"
			"nop;");
	}
#pragma pop_options
#pragma diagnostic pop

}
#endif
