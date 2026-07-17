cmake_minimum_required(VERSION 3.23)
project(controlly C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  set(STRICT_WARNINGS "/W4" "/WX") # Level 4, Treat as Error
  set(RELAXED_WARNINGS "/W3")      # Level 3, ignore third-party noise
else()
  set(STRICT_WARNINGS
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
    "-Wconversion"
    "-Wshadow"
    "-Wdouble-promotion"
    "-Werror=implicit-function-declaration"
    "-fno-strict-aliasing"
    "-Wnull-dereference"
  )
  set(RELAXED_WARNINGS
    "-Wall"
    "-Wextra"
  )
endif()

add_subdirectory(src)

option(CCONTROL_BUILD_EXAMPLES "Build example applications" OFF)

if (CCONTROL_BUILD_EXAMPLES)
  option(CCONTROL_AS_SHOWCASE "Apply showcase features to example applications" OFF)
  add_subdirectory(examples)
endif()

list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing()
add_subdirectory(test)
