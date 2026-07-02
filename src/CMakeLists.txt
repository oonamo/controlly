add_library(controls STATIC)

target_sources(controls
  PRIVATE
    tf.c
    arena.c
    matrix.c
    statespace/mimo.c
    statespace/siso.c
    statespace/router.c
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${PROJECT_SOURCE_DIR}/include
    FILES
      ${PROJECT_SOURCE_DIR}/include/ccontrol/tf.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/arena.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/matrix.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/statespace.h
)
