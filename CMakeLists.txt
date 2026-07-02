cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(controls STATIC)

target_sources(controls
  PRIVATE
    src/tf.c
    src/arena.c
    src/matrix.c
    src/statespace/mimo.c
    src/statespace/siso.c
    src/statespace/router.c
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/ccontrol/tf.h
      include/ccontrol/arena.h
      include/ccontrol/matrix.h
      include/ccontrol/statespace.h
)

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.1
)
FetchContent_MakeAvailable(unity)

find_program(MATLAB_EXE matlab)

if(MATLAB_EXE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/matlab_test_data.h
    COMMAND ${MATLAB_EXE} -batch generate_test_data
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/generate_test_data.m
    COMMENT "Generating MATLAB parity data"
  )
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_generated.c
    COMMAND ${MATLAB_EXE} -batch generate_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/generate_tests.m
    COMMENT "Generating MATLAB parity tests"
  )
else()
  message(WARNING "MATLAB not found in PATH. Using pre-existing matlab files")
endif()

add_executable(test_runner)

target_sources(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src/unity_fixture.c
  test/test_main.c
  test/test_polymath.c
  test/test_transferfunction.c
  test/test_statespace.c
  test/test_generated.c
  test/matlab_test_data.h
)
target_include_directories(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src
)

target_compile_definitions(test_runner PRIVATE UNITY_FIXTURE_NO_EXTRAS)
target_link_libraries(test_runner PRIVATE controls unity)
enable_testing()
add_test(NAME CoreMathTests COMMAND test_runner)
