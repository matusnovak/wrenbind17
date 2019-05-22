cmake_minimum_required(VERSION 3.0)
project(WrenBind17)

option(WRENBIND17_BUILD_TESTS "Build with tests" OFF)
option(WRENBIND17_BUILD_WREN "Build Wren library too" OFF)

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
  enable_testing()
  # Catch2
  set(CATCH2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/Catch2/single_include)

  file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
  set_target_properties(${PROJECT_NAME}_tests PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
  target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CATCH2_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME}_tests PUBLIC Wren ${PROJECT_NAME})
  add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
endif()
