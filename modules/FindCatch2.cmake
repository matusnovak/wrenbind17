include(FindPackageHandleStandardArgs)

if(NOT TARGET Catch2)
  find_path(CATCH2_INCLUDE_DIR NAMES catch2/catch.hpp PATHS ${CMAKE_CURRENT_LIST_DIR}/../libs/Catch2/single_include)
  mark_as_advanced(FORCE CATCH2_INCLUDE_DIR)
  add_library(Catch2 INTERFACE IMPORTED)
  set_target_properties(Catch2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CATCH2_INCLUDE_DIR})
endif()

find_package_handle_standard_args(Catch2 DEFAULT_MSG CATCH2_INCLUDE_DIR)
