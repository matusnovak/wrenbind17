# Call Wren and pass around variables

## Example

Consider the following program below:

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(int argc, char *argv[]) {
    const std::string code = R"(
        class Utils {
            static add(x, b) {
                return x + b
            }
        }
    )";

    // Create new VM
    wren::VM vm;

    // Runs the code from the std::string as a "main" module
    vm.runFromSource("main", code);

    // Find class Main in module main
    wren::Variable mainClass = vm.find("main", "Utils");

    // Find function main() in class Main
    wren::Method add = mainClass.func("add(_, _)");

    // Execute the function
    wren::ReturnValue res = add(10, 20);

    // Checks if the return value is an integer
    // In Wren, integers, floats, longs, doubles, are all the same!
    res.is<int>();         // returns true
    res.is<double>();      // returns true
    res.is<nullptr>();     // returns false
    res.is<std::string>(); // returns false

    // Prints 30
    std::cout << "Result: " << res.as<int>() << std::endl;

    return 0;
}
```

Too call the correct method, you first have to find the class inside of a specific module. When executing a piece of code, you specify the name of the module, in this case `main`. Then, after the code has been executed, all of the classes and variables are defined in the VM of that particular module. So, we first have to find the class. This can be done by calling `wren::VM::find(module_name, class_name)`. Then, we can find any specific function we want by calling `wren::Variable::func(signature)`. The signature you pass into the `func` must match the correct Wren method. Signatures such as `add()` nor `add(_)` won't work, because the Wren function has two arguments. Only `add(_, _)` will work. If the signature is invalid, or the method simply does not exist, an exception `wren::NotFound` is thrown.

Passing variables to Wren is simply done by passing the variables into the `operator(...)`. If the number of variables does not match the signature, it will throw an exception `wren::Exception`. The result of the execution is capured as `wren::ReturnValue`. This can hold anything from nullptr up to a C++ class. To check the contents, simply call the `is<...>()` method, or you get the contents, call the `as<...>()` method. 

::: warning
Warning!

The `wren::Variable`, `wren::Method`, and `wren::ReturnValue` contain handles from Wren that extend the liveness of the variables/methods they hold. You must dispose of them before the VM shuts down. Otherwise the destructor of these classes may cause undefined behavior, even a segmentation fault. Be sure that `wren::VM` is the last thing to be destroyed.
:::

## Types you can pass around

You can pass around any integer between 8 and 64 bits (int8, uint8, int, short, char, unsigned char, etc...), floats, doubles, booleans, and std::strings. These are simple types and will be represented as Wren's built in types. But, Wren has only 64-bit floating point (double). It does not have integers. Therefore, all integers are converted to doubles. Any other types (C++ classes) must be explicitly added to Wren. ([see this guide](bind_cpp_class.md))

::: warning
Warning!

Don't use const char*! It will result in compilation error or it will just thrown a bad cast exception during runtime. Use std::string.
:::
