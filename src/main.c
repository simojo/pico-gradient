#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// helpful information about using state machines
// https://github.com/raspberrypi/pico-feedback/issues/121
// https://dev.to/admantium/raspberry-pico-programming-with-pio-state-machines-1gbg

// T0H | 0 code, high voltage time | 0.4us  | ±150ns
// T1H | 1 code, high voltage time | 0.8us  | ±150ns
// T0L | 0 code, low voltage time | 0.85us | ±150ns
// T1L | 1 code, low voltage time  | 0.45us | ±150ns
// RES | low voltage time          | Above 50µs


static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return
    ((uint32_t)(r) << 8) |
    ((uint32_t)(g) << 16) |
    (uint32_t)(b);
}

int main() {
  stdio_init_all();
  uint pin_number = 1;
  printf("[trace] ws2812b init using pin %d", pin_number);

  PIO pio = pio0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, 0, offset, pin_number, 800000);

  uint num_pixels = 100;
  while (1) {
    for (int i = 0; i < num_pixels; i++) {
      put_pixel(urgb_u32(255, 255, 255));
    }
    sleep_ms(10);
  }
}
