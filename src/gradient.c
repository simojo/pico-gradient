// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// https://www.arrow.com/en/research-and-events/articles/protocol-for-the-ws2812b-programmable-led

#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
  int leds = 100;
  int data[leds][3];
  int pin = 0;
  gpio_init(pin);
  gpio_set_dir(pin, 1);
  while (1) {
    for (int i = 0; i < leds; i++) {
      for (int j = 0; j < 3; j++) {
        int value = data[i][j];
        send_bit(pin,
          0
          || value == 0
          || value == 1
          ? value
          : value % 2);
        value = value >> 1;
      }
    }
  }

  // Tell GPIO 0 and 1 they are allocated to the PWM
  gpio_set_function(0, GPIO_FUNC_PWM);
  gpio_set_function(1, GPIO_FUNC_PWM);

  // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
  uint slice_num = pwm_gpio_to_slice_num(0);

  // Set period of 4 cycles (0 to 3 inclusive)
  pwm_set_wrap(slice_num, 3);
  // Set channel A output high for one cycle before dropping
  pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
  // Set initial B output high for three cycles before dropping
  pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
  // Set the PWM running
  pwm_set_enabled(slice_num, true);

  // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
  // correct slice and channel for a given GPIO.
}

void send_bit(int pin, int high) {
  /* - .8us high, .45us low (1); */
  /* - .4us high, .85us low (0); */
  /* - 300us (reset) */
  /* Every LED sequence is directly followed by the next LED unit's sequence. Eventually, the microcontroller sends a reset low signal (300us or greater) to indicate it's time to start over. The most significant bit goes first, and each signal includes leading zeros to ensure that there are eight bits total per GRB color element. */
  if (high) {
    gpio_put(pin, 1);
    sleep_us(0.8); // FIXME
    gpio_put(pin, 0);
    sleep_us(0.45); // FIXME
  } else {
    gpio_put(pin, 1);
    sleep_us(0.4); // FIXME
    gpio_put(pin, 0);
    sleep_us(0.85); // FIXME
  }
}
