cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(pico_gradient)

pico_sdk_init()

add_executable(main
    main.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_adc hardware_clocks hardware_pio)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
