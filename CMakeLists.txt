cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(controls STATIC)

target_sources(controls
  PRIVATE
    src/tf.c
    src/arena.c
    src/matrix.c
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/ccontrol/tf.h
      include/ccontrol/arena.h
      include/ccontrol/matrix.h
)

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.1
)
FetchContent_MakeAvailable(unity)

add_executable(test_runner test/test_main.c)
target_sources(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src/unity_fixture.c
)
target_include_directories(test_runner PRIVATE
  ${unity_SOURCE_DIR}/extras/fixture/src
)

target_compile_definitions(test_runner PRIVATE UNITY_FIXTURE_NO_EXTRAS)
target_link_libraries(test_runner PRIVATE controls unity)
enable_testing()
add_test(NAME CoreMathTests COMMAND test_runner)
