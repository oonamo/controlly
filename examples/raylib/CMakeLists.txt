set(RAYLIB_VERSION 6.0)
include(FetchContent)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)


FetchContent_MakeAvailable(raylib)
add_executable(example_ss_second_order statespace_second_order.c)

if(CCONTROL_AS_SHOWCASE)
  target_compile_definitions(example_ss_second_order PRIVATE CCONTROL_EXAMPLE_SHOWCASE)
endif()

target_link_libraries(example_ss_second_order PRIVATE controls raylib)
