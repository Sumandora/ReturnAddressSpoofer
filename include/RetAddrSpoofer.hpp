#ifndef RETADDRSPOOFER_HPP
#define RETADDRSPOOFER_HPP

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <sys/mman.h>
#include <unistd.h>

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
		void Protect(const void* addr, const size_t length, const int prot)
		{
			const size_t pagesize = getpagesize();
			void* aligned = (void*)(((size_t)addr) & ~(pagesize - 1));
			const size_t alignDifference = (char*)addr - (char*)aligned;
			mprotect(aligned, alignDifference + length, prot);
		}

		void __attribute((optimize("O0"))) MutateNextCall(void* instruction)
		{
			auto* callInstruction = reinterpret_cast<unsigned char*>(instruction);

			while (true) {
				callInstruction++;

				if (*callInstruction != 0xFF)
					goto nextInstruction;

				for (int index = 6; index <= 12; index++)
					if (*(callInstruction + index) != 0x90)
						goto nextInstruction;

				break; // We found the call-instruction

			nextInstruction:
				continue;
			}

			size_t callRegisterOffset = 1;

			// Call r8-r15 instructions have a 0x41 modifier
			if (*(callInstruction - 1) == 0x41) {
				callInstruction--;
				callRegisterOffset++;
			}

			// Search the first nop instruction
			size_t length = callRegisterOffset + 1 /* Register is one byte */;
			while (*(callInstruction + length) != 0x90)
				length++;

			constexpr int absPushLength = 11;

			Protect(callInstruction, absPushLength + length, PROT_READ | PROT_WRITE | PROT_EXEC);

			// Move call instruction back
			memcpy(callInstruction + absPushLength, callInstruction, length);

			*callInstruction = 0x48; // mov rax, address
			*(callInstruction + 1) = 0xB8;
			*reinterpret_cast<void**>(callInstruction + 2) = leaveRet; // address
			*(callInstruction + 10) = 0x50; // push rax

			// Convert the call instruction to a jmp instruction
			*(callInstruction + absPushLength + callRegisterOffset) += 0x10;

			Protect(callInstruction, absPushLength + length, PROT_READ | PROT_EXEC);
		}

		template <typename Ret, typename... Args>
		Ret __attribute((noinline, optimize("O0"))) _Invoke(void* method, Args... args)
		{
			// This call will later be substituted by push+jmp instructions
			reinterpret_cast<Ret (*)(Args...)>(method)(args...);

			// The compiler warns us about the missing return, but this function won't appear on
			// the call stack and is never returned to.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
			// We need 11 more bytes for the push instruction
			__asm(
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;"
				"nop;");
			// Indirect return, carrying the return value of method
		}
#pragma GCC diagnostic pop
	}

	template <typename Ret, typename... Args>
	static Ret __attribute((noinline)) Invoke(void* method, Args... args)
	{
		static std::atomic_bool mutated = false;
		static std::mutex mutex;

		if (!mutated) {
			if (!leaveRet) {
				// The user hasn't set it yet ._.

				// Alert his debugger if he has one attached
				__asm("int3");
			}
			mutex.lock();
			if (mutated) {
				/*
				 * This is a special case, where one thread called this, started the mutation
				 * Another thread called this function and the mutation wasn't done
				 * The mutex will prevent anymore weird action by the second call, but
				 * we still have to move this thread out of this.
				 * So this weird if-statement has to be here, this is also the reason
				 * why the mutated-variable is being set to true before unlocking the mutex
				 * If someone has a better/cleaner solution to this, please contribute it
				 */
				goto invocation;
			}

			MutateNextCall(reinterpret_cast<void*>(_Invoke<Ret, Args...>));

			mutated = true;
			mutex.unlock(); // Allow other threads to continue
		}

	invocation:

		return _Invoke<Ret, Args...>(method, args...);
	}

}
#endif
