cmake_minimum_required(VERSION 3.0)
project(WrenBind17)

option(WRENBIND17_BUILD_TESTS "Build with tests" OFF)
option(WRENBIND17_BUILD_WREN "Build Wren library too" OFF)

if(WRENBIND17_BUILD_TESTS)
  # Code Coverage Configuration
  add_library(coverage_config INTERFACE)

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(coverage_config INTERFACE
      -O0        # no optimization
      -g         # generate debug info
      --coverage # sets all required flags
    )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
      target_link_options(coverage_config INTERFACE --coverage)
    else()
      target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
  endif()
endif()

add_library(${PROJECT_NAME} INTERFACE)
set(WRENBIND17_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WRENBIND17_BUILD_TESTS OR WRENBIND17_BUILD_WREN)
  # Wren
  set(WREN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/wren/src/include)
  file(GLOB_RECURSE WREN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libs/wren/src/vm/*.c)
  set_source_files_properties(${WREN_SOURCES} PROPERTIES LANGUAGE C)
  add_library(Wren STATIC ${WREN_SOURCES})
  target_include_directories(Wren PRIVATE ${WREN_INCLUDE_DIR})
  target_include_directories(Wren PUBLIC ${WREN_INCLUDE_DIR})
  target_compile_definitions(Wren PRIVATE WREN_OPT_META=0 WREN_OPT_RANDOM=0)
endif()

if(WRENBIND17_BUILD_TESTS)
  target_link_libraries(${PROJECT_NAME} INTERFACE coverage_config)

  enable_testing()
  # Catch2
  set(CATCH2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/Catch2/single_include)

  set(TEST_TARGET ${PROJECT_NAME}_tests)
  file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  add_executable(${TEST_TARGET} ${TEST_SOURCES})
  set_target_properties(${TEST_TARGET} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
  target_include_directories(${TEST_TARGET} PRIVATE ${CATCH2_INCLUDE_DIR})
  target_link_libraries(${TEST_TARGET} PUBLIC Wren ${PROJECT_NAME})
  target_link_libraries(${TEST_TARGET} PRIVATE coverage_config)
  if(MINGW)
    target_compile_options(${TEST_TARGET} PRIVATE -Wa,-mbig-obj)
  endif()
  add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
endif()
