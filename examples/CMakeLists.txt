cmake_minimum_required(VERSION 3.5)

project(erasure_examples CXX)

enable_testing()

add_executable(type_erasure_example type_erasure_example.cpp)
target_link_libraries(type_erasure_example erasure erasure_debug)
add_test(NAME type_erasure_example COMMAND type_erasure_example)
