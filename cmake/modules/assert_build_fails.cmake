# arguments:
# 1: Test name
# 2: target name
# 3: 
# the rest is passed verbatim 
function(assert_build_fails)
  set(one_value_args TEST_NAME TARGET)
  set(multi_value_args DEFINITIONS)
  cmake_parse_arguments(OPT "" "${one_value_args}" "${multi_value_args}"
                        ${ARGN})
  if (NOT DEFINED OPT_TARGET)
    message(FATAL_ERROR "You need to supply the TARGET parameter.")
  endif()
  if (NOT DEFINED OPT_TEST_NAME)
    message(FATAL_ERROR "You need to supply the TEST_NAME parameter.")
  endif()

  add_executable(${OPT_TARGET} ${OPT_UNPARSED_ARGUMENTS})
  # exclude from all builds, because we will invoke the build command in the
  # test
  set_target_properties(${OPT_TARGET} PROPERTIES
                        EXCLUDE_FROM_ALL TRUE
                        EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  target_compile_definitions(${OPT_TARGET} PRIVATE ${OPT_DEFINITIONS})
  add_test(NAME ${OPT_TEST_NAME}
           WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
           COMMAND ${CMAKE_COMMAND} --build . --target ${OPT_TARGET}
                                    --config $<CONFIGURATION>
           )
  set_tests_properties(${OPT_TEST_NAME} PROPERTIES WILL_FAIL TRUE)
endfunction(assert_build_fails)
