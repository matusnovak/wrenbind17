# Tutorial: Customize VM behavior

## Min heap and growth

To control the minimal heap, heap growth, and initial heap, use the constructor to do so. Example:

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(...) {
    // These values are default.
    // If you leave the constructor empty, these exact
    // values will be used.

    // The lookup paths for loading other Wren files
    std::vector<std::string> paths = {"./"};

    // The initial heap that will be created on init
    const size_t initHeap = 1024 * 1024; // 1MB

    // The minimal heap to shrink to
    const size_t minHeap = 1024 * 1024 * 10; // 10MB

    // The growth size
    const int heapGrowth = 50; // 50%

    // Constructor
    wren::VM vm(paths, initHeap, minHeap, heapGrowth);
}

```

## File loader function

When loading other files via `import "hello" for Foo`, then VM will use a file loader function to read the file into a string. This will only happen once per file, even if the file is imported multiple times (Wren controls this). Please note that a function (as shown below) is already provided by default to the `wren::VM`. You don't need to set your own, it's optional. Also, this function does not override the loading of built-in modules (modules you add via `auto& m = vm.module(...);`) but instead this function will get called only after a built-in module with the specific name has not been found. Example:

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(...) {
    auto loadFileFn = [](const std::vector<std::string>& paths, 
                         const std::string& name) -> std::string {
        // The "paths" are lookup paths you use in the wren::VM constructor!
        // The "name" is the name of the import module!
                             
        for (const auto& path : paths) {
            const auto test = path + "/" + std::string(name) + ".wren";

            std::ifstream t(test);
            if (!t)
                continue;

            std::string source(std::istreambuf_iterator<char>(t), 
                               std::istreambuf_iterator<char>());
            return source;
        }

        // To indicate that a file does not exist
        // you must throw eny exception! But, returning
        // an empty string will count as:
        // "file exists but it's empty"!
        throw wren::NotFound();
    };

    wren::VM vm();
    vm.setLoadFileFunc(loadFileFn);
}
```

## Print function

To override the `System.print` simply call the `vm.setPrintFunc` with an appropriate function, example shown below. Please note that if you are printing, for example `System.print("Hello World")`, the print function may be called twice (first time for `Hello World` and second time for `\n`).


```cpp
int main(...) {
    wren::VM vm();
    vm.setPrintFunc([](const char* text) -> void { 
        std::cout << text;
    });
}
```
