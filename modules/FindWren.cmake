include(FindPackageHandleStandardArgs)

if(NOT TARGET Wren)
  find_path(WREN_INCLUDE_DIR NAMES wren.hpp PATHS ${CMAKE_CURRENT_LIST_DIR}/../libs/wren/src/include)
  mark_as_advanced(FORCE WREN_INCLUDE_DIR)

  file(GLOB_RECURSE WREN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libs/wren/src/vm/*.c)
  add_library(Wren STATIC ${WREN_SOURCES}) 
  target_compile_definitions(Wren PRIVATE WREN_OPT_META=0 WREN_OPT_RANDOM=0)
  target_include_directories(Wren PUBLIC ${WREN_INCLUDE_DIR})
  set_target_properties(Wren PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${WREN_INCLUDE_DIR})
endif()

find_package_handle_standard_args(Wren DEFAULT_MSG WREN_INCLUDE_DIR)
