include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.1
)
FetchContent_MakeAvailable(unity)

target_compile_definitions(unity PRIVATE UNITY_OUTPUT_COLOR)

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

target_sources(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src/unity_fixture.c
  test_main.c
  test_polymath.c
  test_transferfunction.c
  test_statespace.c
  test_generated.c
  matlab_test_data.h
)
target_include_directories(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src
)

target_compile_definitions(test_runner PRIVATE UNITY_FIXTURE_NO_EXTRAS)
target_link_libraries(test_runner PRIVATE controls unity)
add_test(NAME UnitTests COMMAND test_runner)
