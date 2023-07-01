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
The return address spoofer works by calling a naked stub function, which will then pop the real return address and push another one. Because GCC does not allow to generate this assembly naturally, we are required to use some hacks involving the rbx register.

## Compatibility
It is compatible with 64-bit GCC  
Clang is not supported due to its unpredictable machine code generation
