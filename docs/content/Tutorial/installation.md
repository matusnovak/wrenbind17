---
title: 'Installation'
weight: 10
---

You don't need to compile this library. This library is a header only and all you have to do is to include the `#include <wrenbind17/wrenbind17.hpp>` header in your C++ project. 

This library will need the Wren main header file: `<wren.hpp>` so make sure your build system has a search path for that header. You will also need to link the Wren library, otherwise you will get linker errors. To see how to build and use the Wren library, see [Getting Started section here](http://wren.io/getting-started.html) from the official Wren documentation.

Optionally, you can build Wren with WrenBind17 by setting CMake flag `WRENBIND17_BUILD_WREN` to `ON`. To do this, simply clone this repository and use CMake in the following way:

```bash
git clone https://github.com/matusnovak/wrenbind17.git
cd wrenbind17
mkdir build
cd build
cmake .. -DWRENBIND17_BUILD_WREN=ON
cmake --build .
```
