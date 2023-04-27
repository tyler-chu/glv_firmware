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
  led_setup();
  led_boot();
  boot_bms();
  switch_setup();
  bms_setup();
  clear_sys_stat();
  spi_setup();
  // check_logs();
  // lcd_setup();
  // lcd_display_ui();
}

void loop()
{
  // TODO: fix checkStatus()
  get_bms_values();
  BMS.checkStatus();
  led_logging();
  // lcd_display_motorvator();
  // bms_setup();
}
