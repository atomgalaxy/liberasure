cmake_minimum_required( VERSION 3.2 )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules/ )

include(CMakeParseArguments)
message(STATUS ${CMAKE_MODULE_PATH})
include(assert_build_fails)

#########################
# function definitions  #
#########################

# add_markdown
# ------------
#
# Add a markdown target that compiles into an html file.
#
# Parameters:
# OPT_TARGET     -- the target being defined (relative to binary dir)
# OPT_SOURCE     -- the main source file (relative to source dir)
# OPT_STYLESHEET -- the html stylesheet (css) that will be used to render the
#                   html document. (relative to source dir)
#
# Other dependencies:
# MD -- the markdown compiler script
#       (has to accept the three arguments in SOURCE, STYLESHEET, TARGET order)
#
set( MD "${CMAKE_SOURCE_DIR}/build_markdown.sh" )
function(add_markdown OPT_TARGET OPT_SOURCE OPT_STYLESHEET)
  set(TARGET "${CMAKE_CURRENT_BINARY_DIR}/${OPT_TARGET}")
  set(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_SOURCE}")
  set(STYLESHEET "${CMAKE_CURRENT_SOURCE_DIR}/${OPT_STYLESHEET}")

  add_custom_command(
    OUTPUT "${TARGET}"
    COMMAND "${MD}" "${SOURCE}" "${STYLESHEET}" "${TARGET}"
    MAIN_DEPENDENCY "${SOURCE}"
    DEPENDS "${STYLESHEET}" "${MD}"
    VERBATIM)
endfunction(add_markdown)
##############################
#  end function definitions  #
##############################


# BUILD TYPE DEFAULTING / PARSING
# If the user specifies -DCMAKE_BUILD_TYPE=BuildType on the command line, take
# their definition and dump it in the cache along with proper documentation,
# otherwise set CMAKE_BUILD_TYPE to Debug prior to calling project(). This is
# the only variable that should be set before the build.
set(CMAKE_BUILD_TYPE_HELP
    "Choose the type of build, options are: "
    " Debug (default), Release, RelWithDebInfo.")
if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" cache string
      "${CMAKE_BUILD_TYPE_HELP}")
else()
   set(CMAKE_BUILD_TYPE "Debug" cache string "${CMAKE_BUILD_TYPE_HELP}")
endif()

project(liberasure)

enable_testing()

#################
# FINDING BOOST #
################# 
# Note: Boost is not required to build the library, as it is meant to be
# completely standalone. It is, however, required to build the tests and the
# lectures.
#
# TODO: enable building lectures only if Boost found.

# add your boost location here in the list of globs, and it should find it
FILE(GLOB BOOST_SEARCH_PATH "/usr/local/Cellar/boost/*")

# make sure versions are sorted properly, so that later versions are ahead of
# the previous ones...
LIST(SORT BOOST_SEARCH_PATH)
LIST(REVERSE BOOST_SEARCH_PATH)

message(STATUS "Looking for Boost in the default places...")
find_package(Boost 1.57.0)
if (NOT Boost_FOUND)
  foreach (BOOST_ROOT ${BOOST_SEARCH_PATH})
    message(STATUS "Looking for Boost in ${BOOST_ROOT}")
    find_package(Boost 1.57.0)
    if (Boost_FOUND)
      break()
    endif()
  endforeach(BOOST_ROOT)
endif()
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found.")
endif()

# include the boost directories
include_directories(${Boost_INCLUDE_DIRS})
# enable c++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -flto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=2" )

# targets:
add_markdown(lesson_plan.html
             lesson_plan/lesson_plan.md
             lesson_plan/codehilite.css)
add_custom_target(lesson_plan ALL DEPENDS lesson_plan.html)
#
include_directories("${CMAKE_SOURCE_DIR}")
add_executable(type_erasure_example       examples/type_erasure_example.cpp)
add_executable(comparisons_example        examples/comparisons_example.cpp)
add_executable(lesson_02                  lectures/lesson_02.cpp)
add_executable(hand_rolled_union          lectures/hand_rolled_union.cpp)
add_executable(lesson_06                  lectures/lesson_06.cpp)
add_executable(lesson_07                  lectures/lesson_07.cpp)
add_executable(lesson_10                  lectures/lesson_10.cpp)
add_executable(lesson_11                  lesson_plan/lesson_11.cpp)
add_executable(mixin_fun                  lectures/mixin_fun.cpp)
add_executable(add_to_ref                 lectures/add_to_ref.cpp)

# movable test & negatives
add_executable(test_type_erasure_movable  tests/test_type_erasure_movable.cpp)
add_test(NAME  test_type_erasure_movable COMMAND test_type_erasure_movable)
assert_build_fails(TEST_NAME negative_test_copyable
                   TARGET negative_test_copyable tests/test_type_erasure_movable.cpp
                   DEFINITIONS NOCOMPILE_COPYABLE_TEST
                   )
assert_build_fails(TEST_NAME negative_test_movable
                   TARGET negative_test_movable tests/test_type_erasure_movable.cpp
                   DEFINITIONS NOCOMPILE_MOVABLE_TEST
                   )

# dereferencable test
add_executable(test_dereferencable        tests/test_dereferencable.cpp)
add_test(NAME  test_dereferencable COMMAND test_dereferencable)

# callable test
add_executable(test_callable              tests/test_callable.cpp)
add_test(NAME  test_callable COMMAND test_callable)

add_test(NAME hand_rolled_union COMMAND hand_rolled_union)
add_test(NAME type_erasure_example_runs COMMAND type_erasure_example)
