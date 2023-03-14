#ifndef BOOT_CONFIG
#define BOOT_CONFIG

#include <Arduino.h>
#include <Wire.h>
#include <BMS_Config.h>
#include <LED_Config.h>

#define BOOT 3

// used to determine if BM IC = ON
bool boot_flag = false;
hw_timer_t *boot_cfg = NULL;

// boot_ISR: called every 3 ms for booting
void IRAM_ATTR boot_ISR(){
    digitalWrite(BOOT, LOW);
}

// boot_setup(): config timer interrupt for booting
void boot_setup(){
    // 40 MHz crystal oscillator - internal system clock
    boot_cfg = timerBegin(0, 40, true);
    timerAttachInterrupt(boot_cfg, &boot_ISR, true);
    timerAlarmWrite(boot_cfg, 3000, true);
    timerAlarmEnable(boot_cfg);
}

// boot_bms: powers on the BM IC using firmware instead of physical push button
void boot_bms(){
    Serial.println("boot_bms(): Running...");

    boot_setup();

    while (!boot_flag) {
        // delay(1000);
        pinMode(BOOT, OUTPUT);
        // delay(1000);

        // attempt to turn on BM IC using BOOT PIN (3)
        digitalWrite(BOOT, HIGH);  // switch diode on
        // delay(3); // 3 ms = 3000 us 
        // digitalWrite(BOOT, LOW);

        // attempt to transmit over I2C communication link (BM IC & ESP-32)
        Wire.beginTransmission(SLAVE_ADDRESS);
        byte error = Wire.endTransmission();

        // if transmission = SUCCESS --> BM IC [ON]
        if (error == 0){
            boot_flag = true;
            Serial.println("- BM IC: [ON]");
            // led_boot();
        }

        // if transmission = FAIl --> BM IC [OFF]
        else
            Serial.println("- BM IC: [OFF]\n");

        delay(2000);
    }
}

#endif 