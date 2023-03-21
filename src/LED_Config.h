#ifndef LED_CONFIG
#define LED_CONFIG

#include <Arduino.h>
#include <Wire.h>
#include <ezButton.h>   // supports button debounce
#include <LCD_CONFIG.h>
#include <SD_Config.h>
#include <BMS_Config.h>

#define LED_RED 11
#define LED_GREEN 13
#define LED_BLUE 15
#define MOMENTARY_SWITCH 1

ezButton button(MOMENTARY_SWITCH);
volatile int led_state = LOW;   // LOW = logging (off), HIGH = logging (on)
volatile int disp_counter = 0;  // used to indicate 1st iteration of a state (terminal output preferences)

// led_setup(): configures required pins as OUTPUT
void led_setup(){
    Serial.println("led_setup(): Running...");
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

// led_boot(): turns RGB LED (blue) when BM IC is powered on/idle state
void led_boot(){
    // display blue light --> ON/IDLE
    if (disp_counter == 0) {
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 0);
        analogWrite(LED_BLUE, 255);

        Serial.println("- BM IC State: [IDLE]");
        Serial.println("- LED Color: [Blue]");
        Serial.println("-----------------------");
    }

    else {
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 0);
        analogWrite(LED_BLUE, 255);
    }
    
    // if (disp_counter != 0)
    //     Serial.println("-----------------------");
}

// led_logging(): turns RGB LED (green) when MOMENTARY SWITCH is flipped to HIGH
void led_logging(){
    get_bms_values();
    // BMS.checkStatus();
    // display on screen
    // update BMS_UI hex array
    // led_display();
 
    // momentary switch (HIGH) --> logging state
    if (led_state == HIGH && disp_counter == 1){
        Serial.println("- BM IC State: [LOGGING]");
        Serial.println("- LED Color: [Green]");
        Serial.println("-----------------------");

        // Logging State: Green 
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 255);
        analogWrite(LED_BLUE, 0);

        // get starting log time 
        set_log_start();
        spi_write(temp_ts1, temp_ts2, current, voltage);

        // used to indicate 1st iteration of a state (terminal output preferences)
        disp_counter += 1;

    }

    else if (led_state == HIGH && disp_counter > 1){

        // Logging State: Green LED
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 255);
        analogWrite(LED_BLUE, 0);

        // write to .csv file, send to micro-sd card
        delay(1000);
        spi_write(temp_ts1, temp_ts2, current, voltage);
    }

    // momentary switch (LOW) --> idle state
    if (led_state == LOW)
        led_boot();

    if (disp_counter == 0)
        disp_counter++; // 2nd iteration of a state (will not print out repetitive message)

}

// led_fault(): turns RGB LED (red) when a circuit fault is detected 
int led_fault(){
    // TODO: implement circuit fault handling functions
    Serial.println("- BM IC State: [CIRCUIT FAULT]");
    Serial.println("- LED Color: [Red]");
    
    // TODO: turn LED Color --> RED
    analogWrite(LED_RED, 255);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);


    return 0;
}

// ISR: momentary switch --> toggle led_state
void IRAM_ATTR toggle_logging(){
    led_state = !led_state;

    counter = 0;        // reset: new file 
    disp_counter = 0;
}

// switch_setup(): configures switch as an input_pullup, sets debounce time
void switch_setup(){
    Serial.println("switch_setup: Running...");
    Serial.println("-----------------------");
    pinMode(MOMENTARY_SWITCH, INPUT_PULLUP);
    button.setDebounceTime(50); // set debounce time to 50 milliseconds
    attachInterrupt(digitalPinToInterrupt(MOMENTARY_SWITCH), toggle_logging, RISING);
}

#endif

