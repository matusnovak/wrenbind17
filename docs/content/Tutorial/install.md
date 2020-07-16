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

## 1.1. Usage with CMake

You can use CMake to add WrenBind17 to your project via [add_subdirectory](https://cmake.org/cmake/help/v3.0/command/add_subdirectory.html). You will need to include WrenBind17 in your project locally (maybe a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)?) Include directories will be handled automatically by [target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html).

```cmake
cmake_minimum_required(VERSION 3.14)
project(Example)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/wrenbind17)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC WrenBind17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF) #c++17
```

{{< hint note >}}
**Note**

You will need to link Wren library too!
{{< /hint >}}


### 1.2. Build with Wren in CMake project

If you wish to build Wren with WrenBind17 (as shown above with `-DWRENBIND17_BUILD_WREN=ON` added to the command line), you can do the following as shown below. Otherwise you will have to build Wren yourself and add it to your CMake project yourself. Setting `WRENBIND17_BUILD_WREN` to `ON` will add `Wren` CMake target which you can use in your own target. Include directories will be handled automatically by [target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html).

```cmake
cmake_minimum_required(VERSION 3.14)
project(Example)

set(WRENBIND17_BUILD_WREN ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/wrenbind17) # Will add Wren target

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC WrenBind17 Wren)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF) #c++17

```
