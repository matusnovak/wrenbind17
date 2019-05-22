cmake_minimum_required(VERSION 3.0)
project(wrenbind17)

set(CMAKE_CXX_STANDARD 17)
option(WRENBIND17_BUILD_TESTS "Build with tests" OFF)

add_library(${PROJECT_NAME} INTERFACE)
set(WRENBIND17_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${WRENBIND17_INCLUDE_DIR})

if(WRENBIND17_BUILD_TESTS)
  enable_testing()
  
  # Wren
  set(WREN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/wren/src/include)
  file(GLOB_RECURSE WREN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libs/wren/src/vm/*.c)
  set_source_files_properties(${WREN_SOURCES} PROPERTIES LANGUAGE C)
  add_library(Wren STATIC ${WREN_SOURCES})
  target_include_directories(Wren PRIVATE ${WREN_INCLUDE_DIR})
  target_compile_definitions(Wren PRIVATE WREN_OPT_META=0 WREN_OPT_RANDOM=0)

  # Catch2
  set(CATCH2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/Catch2/single_include)

  file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
  target_include_directories(${PROJECT_NAME}_tests PRIVATE ${WREN_INCLUDE_DIR} ${CATCH2_INCLUDE_DIR} ${WRENBIND17_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME}_tests PRIVATE Wren ${PROJECT_NAME})
  add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
endif()
