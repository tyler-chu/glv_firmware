// /**
//  * @file main.cpp
//  * @author Tyler, Joey
//  * @brief Firmware for Aztec Electric Racing GLV
//  */

#include <Wire.h>
#include <Arduino.h>
#include <LED_Config.h>
#include <BMS_Config.h>
#include <Boot_Config.h>
#include <SD_Config.h>
#include <LCD_Config.h>

void setup()
{
  Serial.begin(115200);
  delay(3000);
  i2c_setup();
  // i2c_scanner();
  // led_setup();
  boot_bms();
  // switch_setup();
  // bms_setup();
  // clear_sys_stat();
  // spi_setup();
  // lcd_setup();
}

void loop()
{
  // get_bms_values();
  // led_logging();
}

