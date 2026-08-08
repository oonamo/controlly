add_library(controlly STATIC)

option(CONTROLLY_VERBOSE_ERRORS "Enable function names in error callbacks" OFF)

if(CONTROLLY_VERBOSE_ERRORS)
  target_compile_definitions(controlly PUBLIC CONTROLLY_VERBOSE_ERRORS)
endif()

set_target_properties(controlly PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)

if (PROJECT_IS_TOP_LEVEL)
  find_program(IWYU_PATH NAMES include-what-you-use iwyu)

  if(IWYU_PATH)
    set_target_properties(controlly PROPERTIES
      C_INCLUDE_WHAT_YOU_USE "${IWYU_PATH}"
      CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH}"
    )
    message(STATUS "IWYU enabled")
  else()
    message(WARNING "include-what-you-use not found")
  endif()
endif()


target_compile_options(controlly PRIVATE ${STRICT_WARNINGS})

set(CONTROLLY_PRIVATE_SOURCES
    tf.c
    arena.c
    matrix.c
    core.c
    statespace/mimo.c
    statespace/siso.c
    statespace/router.c
    controllers/pid.c
)

set(CONTROLLY_PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/include/controlly.h
    ${PROJECT_SOURCE_DIR}/include/controlly/tf.h
    ${PROJECT_SOURCE_DIR}/include/controlly/arena.h
    ${PROJECT_SOURCE_DIR}/include/controlly/matrix.h
    ${PROJECT_SOURCE_DIR}/include/controlly/statespace.h
    ${PROJECT_SOURCE_DIR}/include/controlly/controllers/pid.h
)

target_sources(controlly
  PRIVATE
    ${CONTROLLY_PRIVATE_SOURCES}
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${PROJECT_SOURCE_DIR}/include
    FILES
      ${CONTROLLY_PUBLIC_HEADERS}
)

target_include_directories(controlly
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

if(PROJECT_IS_TOP_LEVEL)
  set(SINGLE_HEADER_OUTPUT "${CMAKE_BINARY_DIR}/gen/controlly.h")

  add_custom_command(
    OUTPUT ${SINGLE_HEADER_OUTPUT}
    COMMAND python3 ${PROJECT_SOURCE_DIR}/convert_to_headerlib.py
    DEPENDS
    ${PROJECT_SOURCE_DIR}/convert_to_headerlib.py
    ${CONTROLLY_PRIVATE_SOURCES}
    ${CONTROLLY_PUBLIC_HEADERS}

    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating single-header library: dist/controlly.h"
    VERBATIM
  )

  add_custom_target(controlly_single_header ALL DEPENDS ${SINGLE_HEADER_OUTPUT})
endif()

