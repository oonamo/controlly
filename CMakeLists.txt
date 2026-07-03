cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

option(CCONTROL_BUILD_EXAMPLES "Build example applications" OFF)

if (CCONTROL_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

enable_testing()
add_subdirectory(test)
