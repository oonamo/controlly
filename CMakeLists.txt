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
