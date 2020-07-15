---
title: 2. Hello World
---

#  2. Hello World

First, create a `wren::VM` instance, run the source code, find the method you want to run, and execute the method. In the example below, we are printing out a simple "Hello World" to the console. It is recommended that you create an alias from `wrenbind17` to `wren` so you don't have to type that many characters every time. I highly encourage you to **NOT** to use `using namespace wrenbind17;`!

This is the most simple hello world program. The only thing code does is to parse and run a Wren code (a simple print statement) from a simple string. This is not limited to only a single line of code, you can put in an entire contents of some Wren file.

{{< hint info >}}
**Note**

Calling `wren::VM::runFromSource(std::string, std::string)` will parse and run the code at the same time.
{{< /hint >}}

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(int argc, char *argv[]) {
    const std::string code = "System.print(\"Hello World!\")";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    vm.runFromSource("main", "System.print("Hello World!")");

    return 0;
}
```
