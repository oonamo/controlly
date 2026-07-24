add_library(controls STATIC)

set_target_properties(controls PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)

target_compile_options(controls PRIVATE ${STRICT_WARNINGS})

target_sources(controls
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
      ${PROJECT_SOURCE_DIR}/include/ccontrol/tf.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/arena.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/matrix.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/statespace.h
      ${PROJECT_SOURCE_DIR}/include/ccontrol/controllers/pid.h
)
