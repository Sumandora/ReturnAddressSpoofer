#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

namespace RetAddrSpoofer {

	/**
	 * The return address spoofer expects this to be set
	 * This has to be a byte-sequence which contains the following:
	 * For x86:
	 * c9	leave
	 * c3	ret
	 * For x86-64:
	 * c3	ret
	 */
	extern const void* leaveRet;

#ifdef __x86_64
	template <typename... Args>
	void* get_target(Args... /*args*/, void* target)
	{
		return target;
	}

	template <typename Ret, typename... Args>
	__attribute((naked)) Ret inner_invoke(Args... args, void* target)
	{
		// NOLINTNEXTLINE(hicpp-no-assembler)
		asm(
			"call *%0;"
			"push %1;"
			"jmp *%%rax;"
			:
			: "r"(get_target<Args...>), "m"(leaveRet));
	}

	template <typename Ret, typename... Args>
	Ret invoke(void* target, Args... args)
	{
		return inner_invoke<Ret, Args...>(args..., target);
	}
#else
	template <typename Ret, typename... Args>
	__attribute((naked)) Ret inner_invoke(void* target, const void* gadget, Args... args)
	{
		// NOLINTNEXTLINE(hicpp-no-assembler)
		asm("pop %eax;"
			"pop %eax;"
			"jmp *%eax;");
	}

	template <typename Ret, typename... Args>
	__attribute((noinline, force_align_arg_pointer,
#ifdef __clang__
		optnone
#else
		optimize("O0")
#endif
		)) Ret
	invoke(void* target, Args... args)
	{
		return inner_invoke<Ret, Args...>(target, leaveRet, args...);
	}
#endif

}
#endif
