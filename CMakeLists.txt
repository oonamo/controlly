cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
enable_testing()
add_subdirectory(test)
