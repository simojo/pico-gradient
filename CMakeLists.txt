cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(pico_gradient LANGUAGES CXX C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()


add_executable(pico-gradient
    main.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pico-gradient
  pico_stdlib
  hardware_adc
  hardware_i2c
  hardware_clocks
  hardware_pio)

pico_enable_stdio_usb(pico-gradient 1)
pico_enable_stdio_uart(pico-gradient 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pico-gradient)
