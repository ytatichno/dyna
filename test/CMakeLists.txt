cmake_minimum_required (VERSION 2.6)

set (GTEST_PACKAGE_PATH "../gtest" CACHE PATH "Path to the GoogleTest installed package")
set (CMAKE_PREFIX_PATH ${GTEST_PACKAGE_PATH})
find_package(GTest QUIET)

if (NOT GTest_FOUND)
  message (STATUS "Could NOT find Google Test.")
  return ()
else ()
  message (STATUS "Use Google Test.")
endif ()

function (variable_defined var_name)
if (NOT ${var_name})
  message (FATAL_ERROR "${var_name} is not defined")
endif ()
endfunction()
variable_defined (GTEST_INCLUDE_DIR)
variable_defined (GTEST_LIBRARY)
variable_defined (GTEST_MAIN_LIBRARY)

set (SRC
  AddrInfoTest.cpp
  ContextStringParserTest.cpp
  ContextTest.cpp
  string_utils_test.cpp
)

if (MSVC)
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
else ()
  set (CMAKE_CXX_FLAGS "-pthread")
endif ()

add_executable (unit-tests ${SRC})

if (MSVC)
  set_target_properties(unit-tests PROPERTIES FOLDER "${DYNA_FOLDER}")
endif ()

include_directories (unit-tests ${GTEST_INCLUDE_DIR} "../")
target_link_libraries (unit-tests ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} da)
