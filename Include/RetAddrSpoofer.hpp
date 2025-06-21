#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

#include <type_traits>
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

	// NOLINTBEGIN
#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
	// NOLINTEND
	template <typename Ret, typename... Args>
		requires std::conjunction_v<std::negation<std::is_reference<Args>>...>
	Ret __attribute((noinline, force_align_arg_pointer,
#ifdef __clang__
		optnone
#else
		optimize("O0")
#endif
		))
	invoke(void* method, Args... args)
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
		// NOLINTBEGIN
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
		__asm("");
	}
#pragma GCC diagnostic pop
#pragma GCC pop_options
	// NOLINTEND

	template <typename Ret, typename... Args>
		requires std::conjunction_v<std::negation<std::is_reference<Args>>...>
	__attribute((always_inline)) constexpr Ret invoke(Ret (*method)(Args...), Args... args)
	{
		return invoke<Ret, Args...>(reinterpret_cast<void*>(method), std::forward<Args>(args)...);
	}
}
#endif
