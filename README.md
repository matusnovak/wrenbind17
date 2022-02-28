# WrenBind17

[![build](https://github.com/matusnovak/wrenbind17/workflows/build/badge.svg?branch=master)](https://github.com/matusnovak/wrenbind17/actions) [![release](https://img.shields.io/github/v/release/matusnovak/wrenbind17)](https://github.com/matusnovak/wrenbind17/releases) [![codecov](https://codecov.io/gh/matusnovak/wrenbind17/branch/master/graph/badge.svg?token=4DGqb0SlKW)](https://codecov.io/gh/matusnovak/wrenbind17)

WrenBind17 is a C++17 wrapper for [Wren programming language](http://wren.io/). This project was heavily inspired by [pybind11](https://github.com/pybind/pybind11) and by [Wren++](https://github.com/Nelarius/wrenpp). This library is header only and does not need any compilation steps. Simply include the WrenBind17 header `<wrenbind17/wrenbind17.hpp>`, link the Wren library, and you are good to go.

[**https://matusnovak.github.io/wrenbind17**](https://matusnovak.github.io/wrenbind17)

## Features

* Header only.
* Works with Visual Studio 2017/2019 (x64 or x86), MinGW-w64 (x64 or x86), Linux GCC (x64 or arm64), and Apple Clang on Mac OSX (x64).
* C++17 so you don't need to use `decltype()` on class methods to bind them to Wren.
* Foreign modules are automatically generated for you. You don't need to write the extra foreign classes in separate file.
* **Supports strict type safety.** You won't be able to pass just any variable from Wren back to the C++, preventing you getting segmentation faults.
* **Objects are wrapped in std::shared_ptr so you have easier access when passing objects around.** This also enables easy object lifetime management.
* Easy binding system inspired by [pybind11](https://github.com/pybind/pybind11).
* Works with exceptions.
* Upcasting to base types when passing C++ instances.
* Memory leak tested.
* Supports STL containers such as `std::variant`, `std::optional`, `std::vector`, `std::list`, `std::deque`, `std::set`, `std::unordered_set`, `std::map`, `std::unordered_map` which can be handled either natively or as a foreign class.
* Easy binding of operators such as `+`, `-`, `[]`, etc.
* Long but easy to follow tutorial ([here](https://matusnovak.github.io/wrenbind17/tutorial/)).
* Supports native lists and native maps.
* Supports Fn.new{}.
* Supports inheritance (a workaround).
* Supports modularity via look-up paths.
* Supports passing variables by move.

## Limitations

* Passing by a reference to a Wren function will create a copy. Use a pointer if you do not wish to create copies and maintain single instance of a given class. This does not affect C++ member functions that return a reference, in that case it will be treated exactly same as a pointer.
* STL containers `std::unique_ptr`, `std::queue`, `std::stack` are not supported.

## TODO

* Lambdas
* `..`, `...`, and `is` operators
* lcov coverage (currently broken with gcc-9)

## Example

```cpp
#include <wrenbind17/wrenbind17.hpp>
namespace wren = wrenbind17; // Alias

class MyFoo {
public:
    MyFoo(int bar, const std::string& baz);
    
    const std::string& getMessage() const;

    // Properties
    int getYear() const;
    void setYear(int value);

    // Variables
    std::string message;
};

int main(int argc, char *argv[]) {
    const std::string code = R"(
        import "mymodule" for MyFoo, Vec3

        class Main {
            static main() {
                var vec = Vec3.new(1.1, 2.2, 3.3)
                // Do something with "vec"

                var foo = MyFoo.new(2019, "Hello World")
                System.print("Foo type: %(foo.type) ")
                foo.year = 2020
                return foo
            }
        }
    )";

    // Create new VM
    wren::VM vm;
    
    // Create new module
    auto& m = vm.module("mymodule");

    // Add new class
    auto& cls = m.klass<MyFoo>("MyFoo");

    // Add optional constructor
    cls.ctor<int, const std::string&>();

    // Add functions
    cls.func<&MyFoo::getMessage>("getMessage");

    // Define variables
    cls.var<&MyFoo::message>("message"); // Direct access
    cls.prop<&MyFoo::getYear, &MyFoo::setYear>("year"); // As getter & getter
    cls.propReadonly<&MyFoo::getType>("type"); // Read only variable

    // Append some extra stuff to the "mymodule"
    m.append(R"(
        class Vec3 {
            construct new (x, y, z) {
                _x = x
                _y = y
                _z = z
            }
        }
    )");

    // Runs the code from the std::string as a "main" module
    vm.runFromSource("main", code);

    // Find the main() function
    // You can look for classes and their functions!
    auto mainClass = vm.find("main", "Main");
    auto main = mainClass.func("main()");

    auto res = main(); // Execute the function

    // Access the return value
    MyFoo* ptr = res.as<MyFoo*>(); // As a raw pointer
    std::shared_ptr<MyFoo> sptr = res.shared<MyFoo>(); // As a shared ptr

    assert(ptr);
    assert(ptr == sptr.get());

    return 0;
}
```

## Documentation

Tutorials and API documentation can be found here: [https://matusnovak.github.io/wrenbind17/](https://matusnovak.github.io/wrenbind17/)

## Build

There is no need to build this library, it's header only. Simply add the `#include <wrenbind17/wrenbind17.hpp>` and link Wren to your application. That's all.

## Found a bug or want to request a feature?

Feel free to do it on GitHub issues

## Pull requests

Pull requests are welcome

## License

```
MIT License

Copyright (c) 2019-2022 Matus Novak

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

