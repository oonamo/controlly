cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

option(CCONTROL_BUILD_EXAMPLES "Build example applications" OFF)

if (CCONTROL_BUILD_EXAMPLES)
  option(CCONTROL_AS_SHOWCASE "Apply showcase features to example applications" OFF)
  add_subdirectory(examples)
endif()

enable_testing()
add_subdirectory(test)
