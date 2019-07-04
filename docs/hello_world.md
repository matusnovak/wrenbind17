# Hello World

## Simple example

To embed Wren in your application, simply include the `renbind17.hpp` header file, create a `wren::VM` instance, find the method you want to run, and execute the method. In the example below, we are printing out a simple "Hello World" to the console. It is recommended that you create an alias from `wrenbind17` to `wren` so you don't have to type that many characters every time. I highly encourage you to **NOT** to use `using namespace wrenbind17;`!

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(int argc, char *argv[]) {
    const std::string code = R"(
        class Main {
            static main() {
                System.print("Hello World!")
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    vm.runFromSource("main", code);

    // Find class Main in module main
    auto mainClass = vm.find("main", "Main");

    // Find function main() in class Main
    auto main = mainClass.func("main()");

    // Execute the function
    main();

    return 0;
}
```
