cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(pico_gradient LANGUAGES CXX C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_subdirectory(pico-ssd1306)

add_executable(pico-gradient
    main.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pico-gradient
  pico_stdlib
  pico_ssd1306
  hardware_adc
  hardware_i2c
  hardware_clocks
  hardware_pio)

# Include directories for your main executable (if needed)
target_include_directories(pico-gradient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pico-ssd1306)

pico_enable_stdio_usb(pico-gradient 1)
pico_enable_stdio_uart(pico-gradient 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pico-gradient)
