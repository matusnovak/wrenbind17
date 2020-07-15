---
title: 1. Installation
---

# 1. Installation

You don't need to compile this library. **This library is a header only library**, all you have to do is to include the `#include <wrenbind17/wrenbind17.hpp>` header in your C++ project. However, you will need to compile and link the Wren VM, and adding an include path where the `<wren.hpp>` file is located. To see how to build and use the Wren library, see [Getting Started section here](http://wren.io/getting-started.html) from the official Wren documentation.

**Optionally, you can build Wren with WrenBind17** by setting a CMake flag `WRENBIND17_BUILD_WREN` to `ON`. To do this, simply clone this repository and build it in the following way:

```bash
git clone https://github.com/matusnovak/wrenbind17.git wrenbind17
cd wrenbind17
mkdir build

cmake -B ./build -DWRENBIND17_BUILD_WREN=ON .
cmake --build ./build
```
