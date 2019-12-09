# WrenBind17

[![Build Status](https://travis-ci.com/matusnovak/wrenbind17.svg?branch=master)](https://travis-ci.com/matusnovak/wrenbind17) [![Build status](https://ci.appveyor.com/api/projects/status/fy974aj37cdyxc0i/branch/master?svg=true)](https://ci.appveyor.com/project/matusnovak/wrenbind17/branch/master) [![CircleCI](https://circleci.com/gh/matusnovak/wrenbind17.svg?style=svg)](https://circleci.com/gh/matusnovak/wrenbind17) [![codecov](https://codecov.io/gh/matusnovak/wrenbind17/branch/master/graph/badge.svg)](https://codecov.io/gh/matusnovak/wrenbind17)

WrenBind17 is a C++17 wrapper for [Wren programming language](http://wren.io/). This project was heavily inspired by [pybind11](https://github.com/pybind/pybind11) and by [Wren++](https://github.com/Nelarius/wrenpp). This library is header only and does not need any compilation steps. Simply include the `<wrenbind17/wrenbind17.hpp>` header in your application and you are good to go!

## Features

* Header only.
* Works with Visual Studio 2017, MinGW-w64, Linux GCC, and Apple Clang on Mac OSX.
* C++17 so you don't need to use `decltype()` on class methods to bind them to Wren.
* [Foreign modules are automatically generated for you](https://matusnovak.github.io/wrenbind17/docs/tutorial/hello_world.html). You don't need to write the extra foreign classes in separate file.
* **Supports strict type safety.** You won't be able to pass just any variable from Wren back to the C++, preventing you getting segmentation faults.
* Objects are wrapped in `std::shared_ptr` so you have easier access when passing objects around.
* Easy binding system inspired by [pybind11](https://github.com/pybind/pybind11).
* [Works with exceptions](https://matusnovak.github.io/wrenbind17/docs/tutorial/exceptions.html).
* [Upcasting to base types when passing C++ instances](https://matusnovak.github.io/wrenbind17/docs/tutorial/upcasting.html).
* Memory leak tested.
* Supports `std::variant`.
* Supports `std::vector` and `std::list` via helper classes (optional).
* [Easy binding of operators](https://matusnovak.github.io/wrenbind17/docs/tutorial/overload-operators.html) such as `+`, `-`, `[]`, etc.
* [Long but easy to follow tutorial](https://matusnovak.github.io/wrenbind17/docs/tutorial/installation.html).
* [Supports Fn.new{}](https://matusnovak.github.io/wrenbind17/docs/tutorial/callbacks.html).
* [Supports inheritance (a workaround)](https://matusnovak.github.io/wrenbind17/docs/tutorial/inheritance.html).
* [Supports modularity via look-up paths](https://matusnovak.github.io/wrenbind17/docs/tutorial/modules.html).
* [Supports passing variables by move](https://matusnovak.github.io/wrenbind17/docs/tutorial/call-wren.html)

## Limitations

* Passing by a reference to a Wren function will create a copy. Use a pointer if you do not wish to create copies and maintain single instance of a given class. This does not affect C++ member functions that return a reference, in that case it will be treated exactly same as a pointer.

## Not yet implemented

* Lambdas (very tricky due to passing function pointers, most likely not ever be implemented).
* `..`, `...`, and `is` operator binding.
* Helper classes for binding `std::queue`, `std::deque`, `std::stack`, `std::set`, `std::unordered_set`, `std::map`, and `std::unordered_map`.


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

Tutorials can be found [**here**](https://matusnovak.github.io/wrenbind17/docs/tutorial/installation/)

And the autogenerated API documentation via Doxygen [**here**](https://matusnovak.github.io/wrenbind17/docs/modules/group__wrenbind17/)

## Build

There is no need to build this library, it's header only. Simply add the `#include <wrenbind17/wrenbind17.hpp>` and link Wren to your application. That's all.

## Found a bug or want to request a feature?

Feel free to do it on GitHub issues

## Pull requests

Pull requests are welcome

## License

```
MIT License

Copyright (c) 2019 Matus Novak

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

