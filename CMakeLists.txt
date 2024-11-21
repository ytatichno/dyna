cmake_minimum_required (VERSION 3.0)
project (Dyna VERSION 0.4 LANGUAGES CXX)
set (Dyna_DESCRIPTION "Dynamic Analyzer Runtime")
set (Dyna_HOMEPAGE_URL "https://bitbucket.org/dvm-system/dyna")
set (DYNA_FOLDER ""
  CACHE STRING "Specify IDE folder for DYNA Runtime.")

option(USE_JSON_BCL "Use JSON unparser from Base Construction Library (BCL)" OFF)
if (USE_JSON_BCL)
  find_package(BCL 3.0 REQUIRED CONFIG)
  message(STATUS "BCL version: ${BCL_VERSION}")
endif()

set(CMAKE_CXX_STANDARD 17)

set (Dyna_srcs
  Auxiliary.cpp
  Context.cpp
  ContextStringParser.cpp
  DBEntity_VariableType.cpp
  DynamicAnalyser.cpp
  llvm_interface.cpp
  memory_usage.cpp
  omp_dbg.cpp
  Result/AnalysisStorage.cpp
  Result/DependencyAnalysisResult.cpp
)
set (Dyna_headers
  ActualityStorage.h
  Auxiliary.h
  Context.h
  ContextStringParser.h
  DBEntity_VariableType.h
  debug.h
  dyna-config.h.in
  DynamicAnalyser.h
  JsonObjects.h
  memory_usage.h
  m_time.h
  omp_dbg.h
  RegionActualMap.hpp
  type_aliases.hpp
  Result/AnalysisResult.hpp
  Result/AnalysisStorage.hpp
  Result/DependencyAnalysisResult.hpp
  Result/RegionExchangeAnalysisResult.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/dyna-config.h
)
if (USE_JSON_BCL)
  set( Dyna_headers ${Dyna_headers} JsonObjects.h)
endif ()
configure_file ("dyna-config.h.in" "dyna-config.h")
include_directories ("${CMAKE_CURRENT_BINARY_DIR}")

if (MSVC)
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
  set (CMAKE_STATIC_LINKER_FLAGS /NODEFAULTLIB)
endif ()

add_library(da STATIC ${Dyna_srcs} ${Dyna_headers})
if (USE_JSON_BCL)
  target_link_libraries(da BCL::Core)
endif ()

if (MSVC)
  set_target_properties(da PROPERTIES FOLDER "${DYNA_FOLDER}")
  if (USE_JSON_BCL)
    source_group(bcl FILES ${BCL_CORE_HEADERS})
  endif ()
endif ()
if (Dyna_install_path)
  install (TARGETS da DESTINATION ${Dyna_install_path})
endif ()

add_subdirectory (test)
