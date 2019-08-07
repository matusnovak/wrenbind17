# Tutorial: Hello World

First, create a `wren::VM` instance, run the source code, find the method you want to run, and execute the method. In the example below, we are printing out a simple "Hello World" to the console. It is recommended that you create an alias from `wrenbind17` to `wren` so you don't have to type that many characters every time. I highly encourage you to **NOT** to use `using namespace wrenbind17;`!

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
    wren::Variable mainClass = vm.find("main", "Main");

    // Find function main() in class Main
    wren::Method main = mainClass.func("main()");

    // Execute the function
    main();

    return 0;
}
```

::: tip
Note!

Remember to run your script code first! You won't be able to lookup any Wren variables or functions until the script code has been compiled and run via `runFromSource(...)`.
:::

