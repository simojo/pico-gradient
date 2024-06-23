#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "pico-ssd1306/ssd1306.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

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
const int ws2812_pin_number = 0; // GP0: ws2812 bit bashing to neopixel

#define I2C_PORT i2c0
#define I2C_PIN_SDA 4
#define I2C_PIN_SCL 5

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
static struct Hsv hsvReadPrevious = {0, 0, 0};
static struct Hsv hsvReadCurrent = {0, 0, 0};

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
  float x = c * (1 - fabsf(fmodf((h / 60.0), 2) - 1));
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

static inline int mod(float a, float b) {
  float r = fmodf(a, b);
  return r < 0.0 ? r + b : r;
}

int main() {
  printf("starting\n");
  stdio_init_all();
  adc_init();

  adc_gpio_init(26); // GP26_A0: set hue
  adc_gpio_init(27); // GP27_A0: set saturation
  adc_gpio_init(28); // GP28_A0: set brightness
  gpio_init(1); // GP1: toggle hsv1
  gpio_set_dir(1, 0);

  printf("start ssd13066? [y/n]\n");
  char start_ssd1306_char = getchar_timeout_us(5e6);
  if (start_ssd1306_char == 'y') {
    i2c_init(I2C_PORT, 1000000); //Use i2c port with baud rate of 1Mhz
    //Set pins for i2c operation
    gpio_set_function(I2C_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_PIN_SDA);
    gpio_pull_up(I2C_PIN_SCL);

    //Create a new display object
    pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(I2C_PORT, 0x3D, pico_ssd1306::Size::W128xH64);

    //create a vertical line on x: 64 y:0-63
    for (int y = 0; y < 64; y++){
        display.setPixel(64, y);
    }
    display.sendBuffer(); //Send buffer to device and show on screen
  }

  PIO pio = pio0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, 0, offset, ws2812_pin_number, 800000);

  while (1) {
    blink();
    // read hsv input from adc pins & toggle current through each pot rapidly
    hsvReadCurrent = get_hsv_adc();
    // check to see if current input differs from previously cached one
    // if it differs, replace the corresponding hsv with the correct one
    if (1
        && hsvReadCurrent.h != hsvReadPrevious.h
        && hsvReadCurrent.s != hsvReadPrevious.s
        && hsvReadCurrent.v != hsvReadPrevious.v) {
      // hsv input has changed, from previous readout;
      // read switch to decide to change either hsv1 or hsv2
      int changingHsv1 = gpio_get(1);
      int changingHsv2 = !changingHsv1;
      // update corresponding hsv value
      if (changingHsv1) {
        hsv1 = hsvReadCurrent;
      }
      if (changingHsv2) {
        hsv2 = hsvReadCurrent;
      }
    }

    // create an array of rgb pixels
    float deltaH = hsv2.h - hsv1.h;
    // determine which direction (0->360 or 360->0) the hue should fade
    // based upon the difference of the two hues
    if (0) {}
    else if (360 > deltaH && deltaH > 180) {
      deltaH = 360 - deltaH;
    } else if (0 <= deltaH && deltaH <= 180) {
      // do nothing
    } else if (0 > deltaH && deltaH > -180) {
      // do nothing
    } else if (-180 >= deltaH && deltaH >= -360) {
      deltaH = 360 + deltaH;
    }
    // choose steps such that
    // the range [0,num_pixels-1) maps to [hsv1.value,hsv2.value]
    // (hence dividing by (num_pixels - 1), which is the maximum index)
    float h_step = deltaH / (float)(num_pixels - 1);
    float s_step = (hsv2.s - hsv1.s) / (float)(num_pixels - 1);
    float v_step = (hsv2.v - hsv1.v) / (float)(num_pixels - 1);
    struct Hsv hsv_current = hsv1;
    struct Rgb rgbs[num_pixels];
    // iterate along the range [0, num_pixels-1]
    for (int i = 0; i < num_pixels; i++) {
      // set value current rgb value to hsv_current
      rgbs[i] = hsv_to_rgb(hsv_current);
      // then iterate all the values
      hsv_current.h = mod(hsv_current.h + h_step, 360.0);
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
