---
title: 9. Customize VM
---

# 9. Customize VM


## 9.1. Min heap and growth

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

## 9.2. Print function

The print function is defined as:

```cpp
typedef std::function<void(const char*)> PrintFn;
```

and can be set as:

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

int main(...) {
    wren::VM vm(...);
    vm.setPrintFunc([](const char* str) {
        std::cout << str;
    });
}
```

## 9.3. File loader function


You can use your own custom mechanism for handling the imports. This is done by defining your own function of the following type:

```cpp
typedef std::function<std::string(
        const std::vector<std::string>& paths, 
        const std::string& name
    )> LoadFileFn;
```

And using it as this:

```cpp
int main(...) {
    wren::VM vm({"./"});

    const myLoader = [](
        const std::vector<std::string>& paths, 
        const std::string& name) -> std::string {
        
        // "paths" - This list comes from the 
        // first argument of the wren::VM constructor.
        //
        // "name" - The name of the import.

        // Return the source code in this function or throw an exception.
        // For example you can throw wren::NotFound();

        return "";
	};

    vm.setLoadFileFunc(myLoader);
}
```

{{< hint warning >}}
**Warning**

Changing the loader function will also modify the `wren::VM::runFromModule` function. That function depends on the loader. The argument you pass into the `runFromModule` will become the `name` parameter in the loader.
{{< /hint >}}
