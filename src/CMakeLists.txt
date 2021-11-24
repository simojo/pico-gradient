cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(gradient)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(gradient
  gradient.c
)

pico_generate_pio_header(gradient ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(gradient pico_stdlib hardware_pio)

# create map/bin/hex file etc.
pico_add_extra_outputs(gradient)
