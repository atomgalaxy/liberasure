cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
endif()

# liberasure's build breaks if done in-tree. You probably should not build
# things in tree anyway, but we can allow projects that include liberasure as a
# subproject to build in-tree as long as it is not in our tree.
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(
    FATAL_ERROR
      "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt"
  )
endif()

project(
  erasure
  LANGUAGES CXX
  VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(assert_build_fails)

add_library(erasure INTERFACE)
set_target_properties(erasure PROPERTIES INTERFACE_COMPILE_FEATURES cxx_std_20)
target_include_directories(erasure INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(erasure::erasure ALIAS erasure)
target_sources(
  erasure
  INTERFACE erasure/erasure.hpp
            erasure/meta.hpp
            erasure/small_buffer.hpp
            erasure/feature/callable.hpp
            erasure/feature/dereferenceable.hpp
            erasure/feature/equality_comparable.hpp
            erasure/feature/less_than_comparable.hpp
            erasure/feature/ostreamable.hpp
            erasure/feature/regular.hpp
            erasure/feature/value_equality_comparable.hpp)

add_library(erasure_debug INTERFACE)
set_target_properties(erasure_debug PROPERTIES INTERFACE_COMPILE_FEATURES
                                               cxx_std_20)
add_library(erasure_debug::erasure_debug ALIAS erasure_debug)
target_include_directories(erasure_debug INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(
  erasure_debug INTERFACE debug/atom.hpp debug/demangle.hpp
                          debug/instrumented.hpp debug/unique_string.hpp)

add_subdirectory(examples)
add_subdirectory(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
