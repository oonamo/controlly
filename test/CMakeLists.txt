include(FetchContent)
set(UNITY_VERSION 2.7.0)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v${UNITY_VERSION}
)

set(UNITY_EXTENSION_FIXTURE ON CACHE BOOL "Enable Unity Fixtures" FORCE)

FetchContent_MakeAvailable(unity)

find_program(MATLAB_EXE matlab)

if(MATLAB_EXE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/matlab_test_data.h
    COMMAND ${MATLAB_EXE} -batch generate_test_data
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/generate_test_data.m
    COMMENT "Generating MATLAB parity data"
  )
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test_generated.c
    COMMAND ${MATLAB_EXE} -batch generate_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/generate_tests.m
    COMMENT "Generating MATLAB parity tests"
  )
else()
  message(WARNING "MATLAB not found in PATH. Using pre-existing matlab files")
endif()


add_executable(test_runner)

target_compile_definitions(unity
  PUBLIC
  UNITY_OUTPUT_COLOR # Show colored output
  UNITY_INCLUDE_EXEC_TIME # See Unity #838
  UNITY_INCLUDE_COMMAND_LINE_ARGS # Allows for parsing command line arguments
)

target_sources(test_runner PRIVATE
  test_main.c
  test_polymath.c
  test_transferfunction.c
  test_statespace.c
  test_generated.c
  test_pid.c
  matlab_test_data.h
)

target_link_libraries(test_runner PRIVATE controls unity)

# Add verbosity to unit tests
add_test(NAME UnitTests COMMAND test_runner -v)
