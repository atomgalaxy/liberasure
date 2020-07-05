cmake_minimum_required(VERSION 3.5)

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

# define some folders
set(ERASURE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_DIR ${ERASURE_DIR}/test)
set(HEADER_DIR ${ERASURE_DIR}/include)
set(EXAMPLES_DIR ${ERASURE_DIR}/examples)

include(assert_build_fails)

# add catch as a 'linkable' target
add_library(erasure INTERFACE)

# depend on some obvious c++11 features so the dependency is transitively added
# dependents
target_compile_features(
  erasure
  INTERFACE cxx_alignas
            cxx_alignof
            cxx_attributes
            cxx_auto_type
            cxx_constexpr
            cxx_defaulted_functions
            cxx_deleted_functions
            cxx_final
            cxx_lambdas
            cxx_noexcept
            cxx_override
            cxx_range_for
            cxx_rvalue_references
            cxx_static_assert
            cxx_strong_enums
            cxx_trailing_return_types
            cxx_unicode_literals
            cxx_user_literals
            cxx_variadic_macros)

target_include_directories(
  erasure INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(erasure::erasure ALIAS erasure)

# add catch as a 'linkable' target
add_library(erasure_debug INTERFACE)

# depend on some obvious c++11 features so the dependency is transitively added
# dependents
target_compile_features(
  erasure_debug
  INTERFACE cxx_alignas
            cxx_alignof
            cxx_attributes
            cxx_auto_type
            cxx_constexpr
            cxx_defaulted_functions
            cxx_deleted_functions
            cxx_final
            cxx_lambdas
            cxx_noexcept
            cxx_override
            cxx_range_for
            cxx_rvalue_references
            cxx_static_assert
            cxx_strong_enums
            cxx_trailing_return_types
            cxx_unicode_literals
            cxx_user_literals
            cxx_variadic_macros)

add_library(erasure_debug::erasure_debug ALIAS erasure_debug)

target_include_directories(
  erasure_debug INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_subdirectory(examples)
add_subdirectory(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
