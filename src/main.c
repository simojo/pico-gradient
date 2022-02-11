#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

const uint led_pin = 25;
const uint num_pixels = 150;

struct Hsv {
  float h;
  float s;
  float v;
};

struct Rgb {
  uint32_t r;
  uint32_t g;
  uint32_t b;
};

static inline uint32_t urgb_u32(struct Rgb rgb) {
  return
    ((uint32_t)(rgb.r) << 8) |
    ((uint32_t)(rgb.g) << 16) |
    (uint32_t)(rgb.b);
}

static inline struct Rgb hsv_to_rgb(struct Hsv hsv) {
  float h = hsv.h;
  float s = hsv.s;
  float v = hsv.v;
  float c = v * s;
  float x = c * (1 - abs(fmodf((h / 60), 2) - 1));
  float m = v - c;
  float rgb_[3];
  if (false) { }
  else if (0 <= h && h < 60) {
    rgb_[0] = c;
    rgb_[1] = x;
    rgb_[2] = 0;
  }
  else if (0 <= h && h < 60) {
    rgb_[0] = c;
    rgb_[1] = x;
    rgb_[2] = 0;
  }
  else if (60 <= h && h < 120) {
    rgb_[0] = x;
    rgb_[1] = c;
    rgb_[2] = 0;
  }
  else if (120 <= h && h < 180) {
    rgb_[0] = 0;
    rgb_[1] = c;
    rgb_[2] = x;
  }
  else if (180 <= h && h < 240) {
    rgb_[0] = 0;
    rgb_[1] = x;
    rgb_[2] = c;
  }
  else if (240 <= h && h < 300) {
    rgb_[0] = x;
    rgb_[1] = 0;
    rgb_[2] = c;
  }
  else if (300 <= h && h < 360) {
    rgb_[0] = c;
    rgb_[1] = 0;
    rgb_[2] = x;
  }
  struct Rgb result = {
    .r = (uint32_t)((rgb_[0] + m) * 255),
    .g = (uint32_t)((rgb_[1] + m) * 255),
    .b = (uint32_t)((rgb_[2] + m) * 255),
  };
  return result;
}


static inline void blink() {
  gpio_init(led_pin);
  gpio_set_dir(led_pin, 1);

  for (int i = 0; i < 10; i++) {
    gpio_put(led_pin, 1);
    sleep_ms(250);
    gpio_put(led_pin, 0);
    sleep_ms(250);
  }
}

int main() {
  stdio_init_all();

  uint pin_number = 0;

  PIO pio = pio0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, 0, offset, pin_number, 800000);

  // FIXME: pull from pots
  struct Hsv hsv1 = {
    .h = 0.0,
    .s = 1.0,
    .v = 1.0
  };
  struct Hsv hsv2 = {
    .h = 0.0,
    .s = 1.0,
    .v = 1.0
  };

  float delta = hsv2.h - hsv1.h;
  if (0) {}
  else if (360 > delta && delta > 180) {
    delta = 360 - delta;
  } else if (0 <= delta && delta <= 180) {
    // do nothing
  } else if (0 > delta && delta > -180) {
    // do nothing
  } else if (-180 >= delta && delta >= -360) {
    delta = 360 + delta;
  }
  float h_step = delta / num_pixels;
  float s_step = (hsv2.s - hsv1.s) / num_pixels;
  float v_step = (hsv2.v - hsv1.v) / num_pixels;
  struct Hsv hsv_current = hsv1;
  struct Rgb rgbs[num_pixels];
  for (int i = 0; i < num_pixels; i++) {
    rgbs[i] = hsv_to_rgb(hsv_current);
    hsv_current.h += h_step;
    if (hsv_current.h < 0)
      hsv_current.h += 360;
    if (hsv_current.h > 360)
      hsv_current.h -= 360;
    hsv_current.s += s_step;
    hsv_current.v += v_step;
  }

  while (1) {
    for (int i = 0; i < num_pixels; i++) {
      put_pixel(urgb_u32(rgbs[i]));
    }
    sleep_ms(500);
  }
}
