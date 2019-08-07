# Tutorial: Installation

You don't need to compile this library. This library is a header only and all you have to do is to include the `#include <wrenbind17/wrenbind17.hpp>` header in your application. 

This library will additionally need an access to `<wren.hpp>` and you will also need to link the Wren library. To see how to build the Wren library, see [Getting Started section here](http://wren.io/getting-started.html) from the official Wren documentation.

Optionally, you can build WrenBind17 with Wren. To do this, simply clone this repository and use CMake in the following way:

```bash
git clone https://github.com/matusnovak/wrenbind17.git
cd wrenbind17
mkdir build
cd build
cmake .. -DWRENBIND17_BUILD_WREN=ON
cmake --build .
```
