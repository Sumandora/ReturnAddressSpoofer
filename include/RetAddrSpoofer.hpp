#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

namespace RetAddrSpoofer {

	/*
	 * Explanation for leaveRet:
	 * The return address spoofer expects this to be set
	 * This has to be a byte-sequence which contains the following:
	 * c9	leave
	 * c3	ret
	 */
	extern void* leaveRet;

	namespace {
		template <typename Ret, typename... Args>
		__attribute((naked)) inline Ret MutateAddress(Args... args)
		{
			__asm("pop %rax;");
			__asm("push %0;"
				  :
				  : "m"(leaveRet));
			__asm("mov (%rbx),%rax;"
				  "mov 0x8(%rbx),%rbx;"
				  "jmp *%rax;");
		}
	}

	template <typename Ret, typename... Args>
	__attribute((noinline, force_align_arg_pointer, optimize("O0"))) static Ret Invoke(void* method, Args... args)
	{
		struct RBX;
		register RBX* rbx __asm("rbx");
		struct RBX {
			void* a;
			void* b;
		} d{ method, rbx };
		rbx = &d;
		return MutateAddress<Ret, Args...>(args...);
	}

}
#endif
