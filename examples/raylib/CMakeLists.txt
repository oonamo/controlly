set(RAYLIB_VERSION 6.0)
set(RAYGUI_VERSION 4.0)
include(FetchContent)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    raygui
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.tar.gz
)

FetchContent_MakeAvailable(raygui)

set(EXAMPLE_SOURCES
  "statespace_second_order/statespace_second_order.c"
  "pid_leader_follower/pid_leader_follower.c"
)

foreach(SRC_FILE IN LISTS EXAMPLE_SOURCES)
  # Gets filename WE (without extension)
  get_filename_component(TARGET_NAME "${SRC_FILE}" NAME_WE)

  add_executable(${TARGET_NAME} "${SRC_FILE}")

  target_include_directories(${TARGET_NAME} PRIVATE ${raygui_SOURCE_DIR}/src)

  target_compile_options(${TARGET_NAME} PRIVATE ${RELAXED_WARNINGS})

  if(CCONTROL_AS_SHOWCASE)
    target_compile_definitions(${TARGET_NAME} PRIVATE CCONTROL_EXAMPLE_SHOWCASE)
  endif()

  target_link_libraries(${TARGET_NAME} PRIVATE controlly raylib)

  if(EMSCRIPTEN)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".html")

    target_link_options(${TARGET_NAME} PRIVATE
          "-sUSE_GLFW=3"
          "-sASSERTIONS=1"
          "-sWASM=1"
          "-sASYNCIFY"
          "-sGL_ENABLE_GET_PROC_ADDRESS"
          "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
          # "-sINITIAL_MEMORY=67108864" # 64 MB
      )
      # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
      # set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
  endif()
endforeach()
