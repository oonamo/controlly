add_library(controlly STATIC)

set_target_properties(controlly PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)

target_compile_options(controlly PRIVATE ${STRICT_WARNINGS})

target_sources(controlly
  PRIVATE
    tf.c
    arena.c
    matrix.c
    core.c
    statespace/mimo.c
    statespace/siso.c
    statespace/router.c
    controllers/pid.c
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${PROJECT_SOURCE_DIR}/include
    FILES
      ${PROJECT_SOURCE_DIR}/include/controlly/tf.h
      ${PROJECT_SOURCE_DIR}/include/controlly/arena.h
      ${PROJECT_SOURCE_DIR}/include/controlly/matrix.h
      ${PROJECT_SOURCE_DIR}/include/controlly/statespace.h
      ${PROJECT_SOURCE_DIR}/include/controlly/controllers/pid.h
)
