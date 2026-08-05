# This cmake file is mainly for testing the controlly library via a github workflow
# Use as an example for creating your own toolchain
# Requires the avr toolchain

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

# Target the atmega
set(MCU "atmega328p")
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os -ffunction-sections -fdata-sections" CACHE STRING "C flags" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
