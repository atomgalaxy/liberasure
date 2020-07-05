cmake_minimum_required(VERSION 3.5)

project(erasure_tests CXX)

enable_testing()

# movable test & negatives
add_executable(test_type_erasure_movable test_type_erasure_movable.cpp)
target_link_libraries(test_type_erasure_movable erasure)
add_test(NAME test_type_erasure_movable COMMAND test_type_erasure_movable)

add_executable(test_meta test_meta.cpp)
target_link_libraries(test_meta erasure)
add_test(NAME test_meta COMMAND test_meta)

assert_build_fails(
  TEST_NAME
  negative_test_copyable
  TARGET
  negative_test_copyable
  test_type_erasure_movable.cpp
  DEFINITIONS
  NOCOMPILE_COPYABLE_TEST)

assert_build_fails(
  TEST_NAME
  negative_test_movable
  TARGET
  negative_test_movable
  test_type_erasure_movable.cpp
  DEFINITIONS
  NOCOMPILE_MOVABLE_TEST)

# dereferencable test
add_executable(test_dereferencable test_dereferencable.cpp)
target_link_libraries(test_dereferencable erasure)
add_test(NAME test_dereferencable COMMAND test_dereferencable)

# callable test
add_executable(test_callable test_callable.cpp)
target_link_libraries(test_callable erasure)
add_test(NAME test_callable COMMAND test_callable)

