# WrenBind17

[![build](https://github.com/matusnovak/wrenbind17/workflows/build/badge.svg?branch=master)](https://github.com/matusnovak/wrenbind17/actions)

WrenBind17 is a C++17 wrapper for [Wren programming language](http://wren.io/). This project was heavily inspired by [pybind11](https://github.com/pybind/pybind11) and by [Wren++](https://github.com/Nelarius/wrenpp). This library is header only and does not need any compilation steps. Simply include the WrenBind17 header `<wrenbind17/wrenbind17.hpp>`, link the Wren library, and you are good to go.

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
