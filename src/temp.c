// SHAHBAZ HASSAN KHAN MALIK
#include "temp.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

volatile int choice = 2; // determine choice
volatile int res = 9;
volatile char c = 'y';
volatile bool alert_en = false;
volatile float max_temp_val, min_temp_val, min_farhen, max_farhen;
volatile bool blink_active = false;
bool disable = false;
bool mode = false;
volatile int data_core = 1;
// 0 disabled, 1 active low, 2 active high

void alert_func() {

  gpio_init(ALERT);
  gpio_set_dir(ALERT, false);
  gpio_pull_up(ALERT);

  while (1) {

    if (!gpio_get(ALERT)) { // active LOW
      printf("\n[WARNING] TEMPERATURE THRESHOLD EXCEEDED");
      printf("\nPRESS BUTTON6 TO BYPASS SECURITY");
      sleep_ms(67);
      gpio_put(RED, 1);
      sleep_ms(67);
      gpio_put(RED, 0);
      sleep_ms(67);
      gpio_put(RED, 1);
      sleep_ms(67);
    } else {
      gpio_put(RED, 0);
    }
  }
}

void button_callback(uint gpio, uint32_t events) {

  if (gpio == BUTTON) { // read temp
    choice = 1;
    printf("pressed");
  }

  else if (gpio == BUTTON2) { // bus scan
    choice = 2;
    printf("\npressed");
  }

  else if (gpio == BUTTON3) {
    choice = 3; // config menu
    printf("\npressed");
  }

  else if (gpio == BUTTON4) {
    choice = 4; // check device ID
    printf("\npressed");
  }

  else if (gpio == BUTTON5) {
    choice = 5; // alert menu
    printf("\npressed");
  }

  else if (gpio == BUTTON6) {
    printf("\npressed");
    set_temp(false, true);
    set_temp(true, true);
  }
}

void init_button(int *arr, size_t len) { // function to initalize all buttons
  for (int i = 0; i < len; i++) {
    gpio_init(arr[i]);
    gpio_set_dir(arr[i], false);
    gpio_pull_down(BUTTON);

    if (arr[i] == BUTTON) {
      gpio_set_irq_enabled_with_callback(
          arr[i], GPIO_IRQ_EDGE_FALL, true,
          button_callback); // attach interrupt to ALERT pin
    } else {
      gpio_set_irq_enabled(arr[i], GPIO_IRQ_EDGE_RISE,
                           true); // attach interrupt to other buttons
    }
  }
}

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void device_id() {

  uint8_t addr_array[8];

  // scan device id
  printf("Available DEV ID:\n");

  for (int i = 0; i < 8; i++) {
    uint8_t address = TCN75A_ADDRESS | i;
    addr_array[i] = address;

    // does device respond?
    i2c_write_blocking(i2c0, address, NULL, 0, true);
    uint result = i2c_get_hw(i2c0)->status;

    if ((result & I2C_SR_RXNACK_BITS) == 0) {
      printf("[%d] - 0x%02X\n", i, address);
    }
  }

  printf("[X] - QUIT");

  char c2;
  c2 = getchar();

  switch (c2) {
  case '0':
    communicate(addr_array[0]);
    break;

  case '1':
    communicate(addr_array[1]);
    break;

  case '2':
    communicate(addr_array[2]);
    break;

  case '3':
    communicate(addr_array[3]);
    break;

  case '4':
    communicate(addr_array[4]);
    break;

  case '5':
    communicate(addr_array[5]);
    break;

  case '6':
    communicate(addr_array[6]);
    break;

  case '7':
    communicate(addr_array[7]);
    break;

  case 'x':
    choice = 2;
    break;

  default:
    printf("Invalid option! Try again");
  }

  sleep_ms(500);
}

void communicate(uint8_t addr) {
  uint8_t rx_buf;

  i2c_write_blocking(i2c0, addr, NULL, 1, true);
  int ret = i2c_read_blocking(i2c0, addr, &rx_buf, 1, false);

  if (ret == PICO_ERROR_GENERIC) {
    printf("[WARNING] Communication unsuccesful!");
  } else {
    printf("[SUCCESS] Communication successful!");
    blink_seq();
  }
}

void alert_menu() {
  clear();

  printf("\n \t ALERT CONFIG");
  printf("\n[0] \t ALERT ENABLE/DISABLE");
  printf("\n[1] \t SET MAX LIMIT");
  printf("\n[2] \t SET MIN LIMIT");
  printf("\n[3] \t SHOW MAX LIMIT");
  printf("\n[4] \t SHOW MIN LIMIT");
  printf("\n[x] \t QUIT");

  char c2;

  uint8_t limit;

  c2 = getchar();

  switch (c2) {
  case '0':
    alert_en = !alert_en;
    printf("\nALERT %s", alert_en ? "ENABLED" : "DISABLED");
    sleep_ms(1000);
    break;

  case '1': // set max temp limit
    set_temp(true, false);
    sleep_ms(2000);
    alert_menu();
    break;

  case '2': // set min temp limi
    set_temp(false, false);
    sleep_ms(2000);
    alert_menu();
    break;

  case '3': // show max temp limit
    printf("\n Max limit: %f C  |  %f F", max_temp_val, max_farhen);
    sleep_ms(3000);
    alert_menu();
    break;

  case '4': // show min temp limit
    printf("\n Min limit: %f C  |  %f F", min_temp_val, min_farhen);
    sleep_ms(3000);
    alert_menu();
    break;

  case 'x':
    choice = 1;
    break;
  }
}

void alert_triggered() { printf("alert"); }

void read_temp() {

  int data[2] = {TEMP_REGISTER};
  uint8_t temp_addr = 0x00;
  uint8_t reg_addr = CONFIG_REGISTER;
  uint8_t read_buf = read_config();
  uint8_t data_buff[2];
  int16_t temp2;

  i2c_write_blocking(i2c0, TCN75A_ADDRESS, &temp_addr, 1,
                     true); // send 1 byte
  i2c_read_blocking(i2c0, TCN75A_ADDRESS, data, 2,
                    true); // read 2 bytes

  int16_t temp = (data[0] << 8) | data[1];

  float celcius;
  float fahrenheit;

  if (res == 9) {
    temp = (temp >> 7);
    celcius = (float)temp * 0.5f;
    fahrenheit = celcius * 1.8f + 32.0f;
  }

  else if (res == 10) {
    temp = (temp >> 6);
    celcius = (float)temp * 0.25f;
    fahrenheit = celcius * 1.8f + 32.0f;
  }

  else if (res == 11) {
    temp = (temp >> 5);
    celcius = (float)temp * 0.125f;
    fahrenheit = celcius * 1.8f + 32.0f;
  }

  else if (res == 12) {
    celcius = (float)temp / 256.0;
    fahrenheit = celcius * 1.8f + 32.0f;
  }

  printf("\nTemperature: %.6f C | %.6f F\n", celcius, fahrenheit);

  printf("\n");
  print_reg_value(read_buf);

  printf("\nResolution = %d bits", res);

  printf("\nAlert status: %s", data_core != 0 ? "ENABLED" : "DISABLED");

  printf("\nAlert value: %d", gpio_get(ALERT));

  // check temp limit
  uint8_t addr_max = MAX_TEMP;

  i2c_write_blocking(i2c0, TCN75A_ADDRESS, &addr_max, 1,
                     true); // send 1 byte
  i2c_read_blocking(i2c0, TCN75A_ADDRESS, data_buff, 2,
                    false); // read 2 bytes

  temp2 = (data_buff[0] << 8) | data_buff[1];
  temp2 = (temp2 >> 7);

  printf("\nTEMPERATURE THRESHOLD: %f C | %f F", (float)temp2 * 0.5f,
         ((float)temp2 * 0.5f) * 1.8f + 32.0f);

  // sleep_ms(130);
}

void print_reg_value(uint8_t reg) {
  printf("  ONE-SHOT ADC RES   FAULT   ALERT POL COMP/INT SHUTDOWN\n");
  printf("  +--------+--------+--------+--------+--------+--------+\n");
  printf("  |   %d    |   %d%d   |   %d%d   |   %d    |   %d    |   %d    |\n",
         (reg >> 7) & 1, (reg >> 5) & 1, (reg >> 6) & 1, (reg >> 3) & 1,
         (reg >> 4) & 1, (reg >> 2) & 1, (reg >> 1) & 1, (reg >> 0) & 1);
  printf("  +--------+--------+--------+--------+--------+--------+\n");
}

void bus_scan() { // i2c bus scan
  clear();
  printf("\nI2C Bus Scan for Si7021 and VCNL4000\n");
  printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) {
      printf("%02x ", addr);
    }

    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr))
      ret = PICO_ERROR_GENERIC;
    else
      ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

    printf(ret < 0 ? "." : "@");
    printf(addr % 16 == 15 ? "\n" : "  ");
  }

  sleep_ms(2000);
}

void user_interface() { // user interface
  switch (choice) {
  case 1: // bus config
    c = 'y';
    bus_scan();
    break;

  case 2: // read temp
    c = 'y';
    read_temp();
    break;

  case 3:
    config_menu();
    break;

  case 4:
    device_id();
    break;

  case 5:
    alert_menu();
    break;

  default:
    clear();
    printf("f");
  }
}

void blink_seq() {
  for (int i = 0; i < 4; i++) {
    gpio_put(GREEN, 1);
    sleep_ms(500);
    gpio_put(GREEN, 0);
    sleep_ms(500);
  }
}

void init_led(int *arr, size_t len) {
  for (int i = 0; i < len; i++) {
    gpio_init(arr[i]);
    gpio_set_dir(arr[i], true);
  }
}

void shutdown() {

  char c;
  // // char c;
  // // while (1) {
  // //   char c;
  // //   printf("Enter a character: ");
  // //   c = getchar();
  // //   printf("You entered: %c\n", c);
  // // }

  char c2 = '0';
  uint8_t val = read_config(); // current value

  clear();
  printf("\n \t SHUTDOWN SETTING");
  printf("\n[0] \t DISABLE SHUTDOWN");
  printf("\n[1] \t ENABLE SHUTDOWN");
  printf("\n[x] \t RETURN TO MAIN");

  // scanf("%1s", buffer2);
  // printf("%c\n", c2);

  c = getchar();

  if (c == '0') {
    config(0, false); // disable shutdown
    shutdown();
  }

  else if (c == '1') { // enable shutdown
    config(0, true);
    shutdown();
  }

  else if (c == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

void comp() {

  char c2 = '0';

  clear();
  printf("\n \t COMP/INT SETTINGS");
  printf("\n[0] \t COMPARATOR MODE");
  printf("\n[1] \t INTERRUPT MODE");
  printf("\n[x] \t RETURN TO MAIN");

  c2 = getchar();

  if (c2 == '0') { // comparator mode
    config(1, false);
    comp();
  }

  else if (c2 == '1') { // interrupt mode
    config(1, true);
    comp();
  }

  else if (c2 == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

void alert() {

  char c2 = '0';

  clear();
  printf("\n \t ALERT POLARITY");
  printf("\n[0] \t ACTIVE LOW");
  printf("\n[1] \t ACTIVE HIGH");
  printf("\n[x] \t RETURN TO MAIN");

  c2 = getchar();

  if (c2 == '0') {
    config(2, false); // active low

    alert();
  }

  else if (c2 == '1') { // active high
    config(2, true);
    alert();
  }

  else if (c2 == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

void fault() {

  clear();
  char c2 = '0';

  printf("\n \t FAULT QUEUE");
  printf("\n[0] \t 00");
  printf("\n[1] \t 01");
  printf("\n[2] \t 10");
  printf("\n[3] \t 11");
  printf("\n[x] \t RETURN TO MAIN");

  c2 = getchar();

  if (c2 == '0') {
    config(4, false); // 00
    config(3, false);
    fault();
  }

  else if (c2 == '1') {
    config(4, false); // 01
    config(3, true);
    fault();
  }

  else if (c2 == '2') {
    config(4, true); // 10
    config(3, false);
    fault();
  }

  else if (c2 == '3') {
    config(4, true); // 11
    config(3, true);
    fault();
  }

  else if (c2 == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

void adc() {
  clear();

  char c2 = '0';

  printf("\n \t ADC RESOLUTION");
  printf("\n[0] \t 9 BIT 0R 0.5C");
  printf("\n[1] \t 10 BIT OR 0.25C");
  printf("\n[2] \t 11 BIT OR 0.125C");
  printf("\n[3] \t 12 BIT OR 0.0625C");
  printf("\n[x] \t RETURN TO MAIN");

  c2 = getchar();

  if (c2 == '2') {    // 11 bits
    config(6, true);  // 1x
    config(5, false); // 10
    res = 11;
    adc();
  } else if (c2 == '0') { // 9 bits
    config(6, false);     // 0x
    config(5, false);     // 00
    res = 9;
    adc();
  } else if (c2 == '3') { // 12 bits
    config(6, true);      // 1x
    config(5, true);      // 11
    res = 12;
    adc();
  } else if (c2 == '1') { // 10 bits
    config(6, false);     // 1x
    config(5, true);      // 11
    res = 10;
    adc();
  } else if (c2 == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

uint8_t read_config() {
  uint8_t read_buf;
  uint8_t reg_addr = CONFIG_REGISTER;

  i2c_write_blocking(i2c0, TCN75A_ADDRESS, &reg_addr, 1, true);

  i2c_read_blocking(i2c0, TCN75A_ADDRESS, &read_buf, 1, false);

  return read_buf;
}

void one_shot() {
  clear();

  char c2 = '0';

  printf("\n \t ONE SHOT SETTING");
  printf("\n[0] \t ENABLE");
  printf("\n[1] \t DISABLE");
  printf("\n[x] \t RETURN TO MAIN");

  c2 = getchar();

  if (c2 == '0') {
    config(7, true);
    one_shot();
  } else if (c2 == '1') {
    config(7, false);
    one_shot();
  } else if (c2 == 'x') {
    c = 'y';
    config_menu();
  }

  sleep_ms(500);
}

void config(uint8_t bit_pos, bool new_value) {

  printf("\nbefore: 0x%X", read_config());

  uint8_t reg_addr;
  reg_addr = CONFIG_REGISTER;
  uint8_t read_buf;

  uint8_t reg_val = read_config();

  // Set or clear the bit at the specified position
  if (new_value) {
    reg_val |= (1 << bit_pos);
  } else {
    reg_val &= ~(1 << bit_pos);
  }

  uint8_t buf[] = {reg_addr, reg_val};
  i2c_write_blocking(i2c0, TCN75A_ADDRESS, buf, 2, false);

  // Read back the register to verify the change was successful
  i2c_write_blocking(i2c0, TCN75A_ADDRESS, &reg_addr, 1, true);

  i2c_read_blocking(i2c0, TCN75A_ADDRESS, &read_buf, 1, false);

  printf("\nafter: 0x%X", read_buf);

  if (reg_val == read_buf) {
    printf("\n Configuration successful!");
    blink_seq();
  }
}

void set_temp(bool choice, bool reset) {

  int limit;
  uint8_t config_data[3];
  char hex_string[3];

  if (!reset) {
    printf("\nEnter the temperature limit (in decimal): ");
    scanf("%d", &limit);
  } else {
    limit = 120;
  }

  if (choice) {
    config_data[0] = MAX_TEMP; // set max temp or min temp?
  } else {
    config_data[0] = MIN_TEMP; // set min temp
  }

  sprintf(hex_string, "%02x", limit); // int to hex

  config_data[1] = (uint8_t)strtol(hex_string, NULL, 16);
  config_data[2] = 0x00;

  i2c_write_blocking(i2c0, TCN75A_ADDRESS, config_data, 3, true);
  uint8_t data[2];
  i2c_read_blocking(i2c0, TCN75A_ADDRESS, data, 2, false);

  int16_t temp = (data[0] << 8) | data[1];
  temp >>= 7;

  float celcius = temp * 0.5f;

  if (!reset) {
    if (celcius == limit) {
      printf("Temperature limit set successfuly");
    } else {
      printf("Failed to set temperature limit");
    }
  }

  float fahrenheit = celcius * 1.8f + 32.0f;

  if (choice) {
    max_temp_val = celcius;
    max_farhen = fahrenheit;
  }

  else {
    min_temp_val = celcius;
    min_farhen = fahrenheit;
  }

  if (!reset) {
    printf("\nTemp limit: %f C", celcius);
    printf("\nTemp limit: %f F", fahrenheit);
  } else {
    printf("\nSECURITY MEASURE BYPASSED");
  }
}

void config_menu() {
  clear();

  char c;

  printf("\n[0] \t SHUTDOWN SETTINGS");
  printf("\n[1] \t COMP/INT SELECT");
  printf("\n[2] \t ALERT POLARITY");
  printf("\n[3] \t FAULT QUEUE");
  printf("\n[4] \t ADC RES");
  printf("\n[5] \t ONE-SHOT");
  printf("\n[X] \t QUIT");

  c = getchar();

  switch (c) {

  case '0':
    clear();
    shutdown();
    break;

  case '1':
    clear();
    comp();
    break;

  case '2':
    clear();
    alert();
    break;

  case '3':
    clear();
    fault();
    break;

  case '4':
    clear();
    adc();
    break;

  case '5':
    clear();
    one_shot();
    break;

  case 'y': // main menu
    clear();
    printf("\n[0] \t SHUTDOWN SETTINGS");
    printf("\n[1] \t COMP/INT SELECT");
    printf("\n[2] \t ALERT POLARITY");
    printf("\n[3] \t FAULT QUEUE");
    printf("\n[4] \t ADC RES");
    printf("\n[5] \t ONE-SHOT");
    printf("\n[X] \t QUIT");
    break;

  case 'x':
    choice = 2;
    break;

  default:
    printf("\ninvalid option, choose again!");
  }

  sleep_ms(500);
}

void init_i2c() {
  i2c_init(i2c0, 400000);
  gpio_set_function(1, GPIO_FUNC_I2C); // set GPIO0 as I2C SCL
  gpio_set_function(0, GPIO_FUNC_I2C); // set GPIO1 as I2C SDA
  gpio_pull_up(0);
  gpio_pull_up(1);
}
