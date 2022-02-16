cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/modules")

project(WrenBind17)

option(WRENBIND17_BUILD_TESTS "Build with tests" OFF)
option(WRENBIND17_BUILD_WREN "Build Wren library too" OFF)
option(WRENBIND17_COVERAGE "Enable coverage reporting" OFF)

# Add WrenBind17 header only library
add_library(${PROJECT_NAME} INTERFACE)
set(WRENBIND17_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${WRENBIND17_INCLUDE_DIR})

if(WRENBIND17_BUILD_TESTS OR WRENBIND17_BUILD_WREN)
  # Find Wren library
  find_package(Wren REQUIRED)
endif()

if(WRENBIND17_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Code Coverage Configuration
  add_library(${PROJECT_NAME}_Coverage INTERFACE)

  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(${PROJECT_NAME}_Coverage INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(${PROJECT_NAME}_Coverage INTERFACE --coverage)
  else()
    target_link_libraries(${PROJECT_NAME}_Coverage INTERFACE --coverage)
  endif()
endif()

if(WRENBIND17_BUILD_TESTS)
  # Find Catch2 library
  find_package(Catch2 REQUIRED)

  # Add tests
  enable_testing()
  file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  file(GLOB TEST_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp)
  file(GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/wrenbind17/*.hpp)
  add_executable(${PROJECT_NAME}_Tests ${TEST_SOURCES} ${TEST_HEADERS} ${LIB_HEADERS})
  set_target_properties(${PROJECT_NAME}_Tests PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
  target_include_directories(${PROJECT_NAME}_Tests PRIVATE ${CATCH2_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME}_Tests PUBLIC Wren ${PROJECT_NAME})
  if(UNIX AND NOT APPLE)
    # Coverage info
    target_compile_options(${PROJECT_NAME}_Tests PRIVATE --coverage -g -O0)
    target_link_options(${PROJECT_NAME}_Tests PRIVATE --coverage)
  endif()
  if(MINGW)
    target_compile_options(${PROJECT_NAME}_Tests PRIVATE -Wa,-mbig-obj)
  endif()
  add_test(NAME ${PROJECT_NAME}_Tests COMMAND ${PROJECT_NAME}_Tests)

  if(WRENBIND17_COVERAGE)
    target_link_libraries(${PROJECT_NAME}_Tests PUBLIC ${PROJECT_NAME}_Coverage)
  endif()
endif()

# install headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/wrenbind17"
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hxx"
  PATTERN "*.hpp"
  PATTERN "*.h"
)