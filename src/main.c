#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// helpful information about using state machines
// https://github.com/raspberrypi/pico-feedback/issues/121
// https://dev.to/admantium/raspberry-pico-programming-with-pio-state-machines-1gbg

// T0H | 0 code, high voltage time | 0.4us  | ±150ns
// T1H | 1 code, high voltage time | 0.8us  | ±150ns
// T0L | 0 code, low voltage time  | 0.85us | ±150ns
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

static struct Hsv hsv1 = {0, 0, 0};
static struct Hsv hsv2 = {0, 0, 0};

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
  float x = c * (1 - abs(fmodf((h / 60.0), 2) - 1));
  float m = v - c;
  float rgb_[3];
  if (0) {
  } else if (0 <= h && h < 60) {
    rgb_[0] = c;
    rgb_[1] = x;
    rgb_[2] = 0;
  } else if (60 <= h && h < 120) {
    rgb_[0] = x;
    rgb_[1] = c;
    rgb_[2] = 0;
  } else if (120 <= h && h < 180) {
    rgb_[0] = 0;
    rgb_[1] = c;
    rgb_[2] = x;
  } else if (180 <= h && h < 240) {
    rgb_[0] = 0;
    rgb_[1] = x;
    rgb_[2] = c;
  } else if (240 <= h && h < 300) {
    rgb_[0] = x;
    rgb_[1] = 0;
    rgb_[2] = c;
  } else if (300 <= h && h < 360) {
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

static inline struct Hsv get_hsv_adc() {
  // NOTE: division with integers will auto round, must multiply by 360 before deviding by 4096
  adc_select_input(0); // gp26
  uint16_t h = ((uint16_t)adc_read()) * 360 / (1 << 12);
  adc_select_input(1); // gp27
  uint16_t s = ((uint16_t)adc_read()) * 360 / (1 << 12);
  adc_select_input(2); // gp28
  uint16_t v = ((uint16_t)adc_read()) * 360 / (1 << 12);
  struct Hsv result = {
    .h = h,
    .s = s,
    .v = v,
  };
  return result;
}

static inline void blink() {
  gpio_init(led_pin); // led
  gpio_set_dir(led_pin, 1);

  for (int i = 0; i < 5; i++) {
    gpio_put(led_pin, 1);
    sleep_ms(100);
    gpio_put(led_pin, 0);
    sleep_ms(100);
  }
}

int main() {
  stdio_init_all();
  adc_init();
  gpio_init(26); // adc0
  gpio_init(27); // adc1
  gpio_init(28); // adc2
  gpio_init(1); // toggle hsv1
  gpio_set_dir(1, 0);

  uint pin_number = 0;

  PIO pio = pio0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, 0, offset, pin_number, 800000);

  while (1) {
    blink();
    // read hsv input from adc pins & toggle current through each pot rapidly
    /* struct Hsv temphsv = get_hsv_adc();
    // check to see if current input differs from other side
    // if it differs, replace the corresponding hsv with the correct one
    if (gpio_get(1)) {
      // if switch is toggled to the left (on)
      if (1
        && temphsv.h == hsv2.h
        && temphsv.s == hsv2.s
        && temphsv.v == hsv2.v) {
        // do nothing
      } else {
        hsv1 = temphsv;
      }
    } else {
      // if switch is toggled to the right (off)
      if (1
        && temphsv.h == hsv1.h
        && temphsv.s == hsv1.s
        && temphsv.v == hsv1.v) {
        // do nothing
      } else {
        hsv2 = temphsv;
      }
    } */

    hsv1.h = 0;
    hsv1.s = 79;
    hsv1.v = 78;

    hsv2.h = 261;
    hsv2.s = 79;
    hsv2.v = 78;

    // create an array of rgb pixels
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
      // FIXME: remove line below
      hsv_current.h = i * 2;
      hsv_current.s = 100;
      hsv_current.v = 60;
      rgbs[i] = hsv_to_rgb(hsv_current);
      /* rgbs[i].r = 107; */
      /* rgbs[i].g = 24; */
      /* rgbs[i].b = 135; */
      hsv_current.h += h_step;
      if (hsv_current.h < 0)
        hsv_current.h += 360;
      if (hsv_current.h > 360)
        hsv_current.h -= 360;
      hsv_current.s += s_step;
      hsv_current.v += v_step;
    }
    // push the pixels sequentially out of the stack
    for (int i = 0; i < num_pixels; i++) {
      put_pixel(urgb_u32(rgbs[i]));
    }
    sleep_ms(1000);
  }
}
