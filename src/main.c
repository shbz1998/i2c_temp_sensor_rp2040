#include "temp.h"
#include <stdio.h>

int main(void) {

  stdio_init_all();

  clear();

  int button[] = {BUTTON, BUTTON2, BUTTON3, BUTTON4, BUTTON5, BUTTON6};
  int led[] = {RED, GREEN};

  size_t nelms_led = sizeof(led) / sizeof(led[0]);
  size_t nelms = sizeof(button) / sizeof(button[0]);

  init_button(button, nelms);
  init_led(led, nelms_led);

  char buffer[1];

  init_i2c();

  multicore_launch_core1(alert_func);

  while (1) {
    tight_loop_contents();
    sleep_ms(50);
    clear();
    user_interface();
  }

  return 0;
}
