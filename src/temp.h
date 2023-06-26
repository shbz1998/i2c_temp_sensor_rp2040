#ifndef TEMP_H__
#define TEMP_H__

#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define TCN75A_ADDRESS 0x48  // I2C address of TCN75A temperature sensor
#define TEMP_REGISTER 0x00   // Temperature register of TCN75A
#define CONFIG_REGISTER 0x01 // Configuration register of TCN75A
#define MIN_TEMP 0x02        // HYSTERSIS TEMP REGISTER
#define MAX_TEMP 0x03        // SET TEMP REGISTER

#define I2C_SR_RXNACK_BITS (1u << 0)

#define clear() printf("\033[H\033[J") // clearing the serial terminal

#define BUTTON 16
#define BUTTON2 19
#define BUTTON3 17
#define BUTTON4 20
#define BUTTON5 21
#define BUTTON6 22

#define RED 13
#define GREEN 9

#define ALERT 15

// main functions
void button_callback(uint gpio, uint32_t events);
void init_button(int *arr, size_t len);
void init_led(int *arr, size_t len);
bool reserved_addr(uint8_t addr);
void bus_scan();
void user_interface();
void init_i2c();
void read_temp();
void blink_seq();
void reset_alert();

// optional configuration functions
void config_menu();
void shutdown();
void comp();
void alert();
void fault();
void adc();
void one_shot();
void config(uint8_t bit_pos, bool new_value);
uint8_t read_config();
void print_reg_value(uint8_t reg);
void device_id();
void communicate(uint8_t addr);
void alert_menu();
void set_temp(bool choice, bool reset);
void alert_func();

#endif