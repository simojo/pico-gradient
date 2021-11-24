// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// https://www.arrow.com/en/research-and-events/articles/protocol-for-the-ws2812b-programmable-led

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

const uint32_t NUM_PIXELS = 100;
const uint32_t PIN = 0;

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t rgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  // change to grb with each color corresponding to 8 bit reg
  return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | ((uint32_t)(b));
}

int main() {
  stdio_init_all();
  PIO pio = pio0;
  int sm = 0;
  uint offset = pio_add_program(pio, &ws2812_program);
  bool IS_RGBW = true;

  ws2812_parallel_program_init(pio, sm, offset, PIN, 800000, IS_RGBW);

  put_pixel(rgb_u32(200, 10, 100));
}
