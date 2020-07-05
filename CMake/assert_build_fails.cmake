# Copyright 2015, 2016 Gašper Ažman
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

include(CMakeParseArguments)

# arguments: 1: Test name 2: target name 3: the rest is passed verbatim
function(assert_build_fails)
  set(one_value_args TEST_NAME TARGET)
  set(multi_value_args DEFINITIONS)
  cmake_parse_arguments(OPT "" "${one_value_args}" "${multi_value_args}"
                        ${ARGN})
  if(NOT DEFINED OPT_TARGET)
    message(FATAL_ERROR "You need to supply the TARGET parameter.")
  endif()
  if(NOT DEFINED OPT_TEST_NAME)
    message(FATAL_ERROR "You need to supply the TEST_NAME parameter.")
  endif()

  add_executable(${OPT_TARGET} ${OPT_UNPARSED_ARGUMENTS})
  # exclude from all builds, because we will invoke the build command in the
  # test
  set_target_properties(
    ${OPT_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD
                                                   TRUE)
  target_compile_definitions(${OPT_TARGET} PRIVATE ${OPT_DEFINITIONS})
  add_test(
    NAME ${OPT_TEST_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --target ${OPT_TARGET} --config
            $<CONFIGURATION>)
  set_tests_properties(${OPT_TEST_NAME} PROPERTIES WILL_FAIL TRUE)
endfunction(assert_build_fails)
