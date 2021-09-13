cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(PICO_SDK_FETCH_FROM_GIT on)

project(gradient)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(gradient
  gradient.c
)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(gradient pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(gradient)
