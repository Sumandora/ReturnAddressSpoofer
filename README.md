# Return Address Spoofer
This is a thread-safe, header-only return address spoofer for x86-64 linux systems using the SystemV ABI.

## Usage
An example can be found in the Source folder.  

A minimal example could look like this:
```c++
#include "RetAddrSpoofer.hpp"

void TestFunction(int arg1) {
	// ...
}

void* RetAddrSpoofer::leaveRet = returnGadget; // Pointer to 0xC9C3 (leave & ret)

int main() {
	RetAddrSpoofer::Invoke<void, int>(reinterpret_cast<void*>(TestFunction), 1337);
}
```

## Compiling the example
```bash
mkdir Build && cd Build

cmake ..
make
./ReturnAddressSpoofer
```

## How it works
The return address spoofer works by replacing the call instruction to the target function with a push and jmp instruction. The call instruction pushes the return address on the stack and then jmps to the target function, by replacing the call instruction we get the freedom to specify anything as the return address.

## Compatibility
It is compatible with 64-bit GCC  
Clang is not supported due to its unpredictable machine code generation