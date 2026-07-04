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

target_compile_options(example_ss_second_order PRIVATE ${RELAXED_WARNINGS})

if(CCONTROL_AS_SHOWCASE)
  target_compile_definitions(example_ss_second_order PRIVATE CCONTROL_EXAMPLE_SHOWCASE)
endif()

target_link_libraries(example_ss_second_order PRIVATE controls raylib)

if(EMSCRIPTEN)
    set_target_properties(example_ss_second_order PROPERTIES SUFFIX ".html")

    target_link_options(example_ss_second_order PRIVATE
        "-sUSE_GLFW=3"
        "-sASSERTIONS=1"
        "-sWASM=1"
        "-sASYNCIFY"
        "-sGL_ENABLE_GET_PROC_ADDRESS"
        # "-sINITIAL_MEMORY=67108864" # 64 MB
    )
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    # set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()
