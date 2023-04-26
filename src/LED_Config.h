#ifndef LED_CONFIG
#define LED_CONFIG

#include <Arduino.h>
#include <Wire.h>
#include <ezButton.h>   // supports button debounce
#include <LCD_CONFIG.h>
#include <SD_Config.h>
#include <BMS_Config.h>
#include <bq769x0.h>
#include <string>

#define LED_RED 11
#define LED_GREEN 13
#define LED_BLUE 15
#define MOMENTARY_SWITCH 1

// circuit fault macros
#define XREADY1 32
#define XREADY2 160
#define UV1 8
#define UV2 136
#define OV1 4
#define OV2 132
#define SCD1 2
#define SCD2 130
#define UTEMP 254
#define OTEMP 255
// #define OCD1 1
// #define OCD2 129

ezButton button(MOMENTARY_SWITCH);
volatile int led_state = LOW;   // LOW = logging (off), HIGH = logging (on)
volatile int disp_counter = 0;  // used to indicate 1st iteration of a state (terminal output preferences)
// int fault_counter = 0;
char fault_name[50];

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

// led_fault(): turns RGB LED (red) when a circuit fault is detected 
void led_fault(){
    analogWrite(LED_RED, 255);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
}

// fault_checker(): diagnose which fault is present 
void fault_checker(uint8_t regByte){

    switch(regByte) {
        case XREADY1:
            Serial.println("- XR Error [y]");
            strcpy(fault_name, "XR Error");
            break;
        case XREADY2:
            Serial.println("- XR Error [y]");
            strcpy(fault_name, "XR Error");
            break;
        case UV1:
            Serial.println("- UV Error [y]");
            strcpy(fault_name, "UV Error");
            break;
        case UV2:
            Serial.println("- UV Error [y]");
            strcpy(fault_name, "UV Error");
            break;
        case OV1:
            Serial.println("- OV Error [y]");
            strcpy(fault_name, "OV Error");
            break;
        case OV2:
            Serial.println("- OV Error [y]");
            strcpy(fault_name, "OV Error");
            break;
        case SCD1:
            Serial.println("- SCD Error [y]");
            strcpy(fault_name, "SCD Error");
            break;
        case SCD2:
            Serial.println("- SCD Error [y]");
            strcpy(fault_name, "SCD Error");
            break;
        // case OCD1:
        //     Serial.println("- OCD Error [y]");
        //     break;
        // case OCD2:
        //     Serial.println("- OCD Error [y]");
        //     break;
        }
    
    if (BMS.TEMP_FAULT){
        Serial.println("- Temperature Error [y]");
        strcpy(fault_name, "Temp Error");
    }

    if (BMS.OV_FLAG){
        Serial.println("- OV Error [y]");
        strcpy(fault_name, "OV Error");
    }

    if (BMS.UV_FLAG){
        Serial.println("- UV Error [y]");
        strcpy(fault_name, "UV Error");
    }
}

// fault_detection(): check to see if a fault is present 
void fault_detection(uint8_t regByte){
    
    // check if a fault is present 
    if (regByte == XREADY1 || regByte == XREADY2 || regByte == UV1 || regByte == UV2 || regByte == OV1 || regByte == OV2 || regByte == SCD1 || regByte == SCD2 || (BMS.TEMP_FAULT == true) || (BMS.OV_FLAG == true) || (BMS.UV_FLAG == true)){

        // if fault, always log
        led_state = 1;

        led_fault();    // turn LED red 

        // 1st iteration of fault being present 
        if (BMS.fault_counter == 1){
            Serial.println("- BM IC State: [FAULT]");
            Serial.println("- LED Color: [RED]\n");
            Serial.print("SYS_STAT: ");
            Serial.println(regByte);
            fault_checker(regByte);
            Serial.println("-----------------------");
        }

        BMS.FAULT_FLAG = true;  // do not collect data until fault is resolved 
    }
}


// led_logging(): turns RGB LED (green) when MOMENTARY SWITCH is flipped to HIGH
void led_logging(){

    regSYS_STAT_t fault_reg;
    fault_reg.regByte = BMS.readRegister(SYS_STAT);
    BMS.fault_counter++;

    // fault detection
    fault_detection(fault_reg.regByte);
    // if (BMS.FAULT_FLAG)
    //     return;

    // momentary switch (HIGH) --> logging state
    if (led_state == HIGH && disp_counter == 1){
        if (!BMS.FAULT_FLAG){
            Serial.println("- BM IC State: [LOGGING]");
            Serial.println("- LED Color: [Green]");
            Serial.println("-----------------------");

            analogWrite(LED_RED, 0);
            analogWrite(LED_GREEN, 255);
            analogWrite(LED_BLUE, 0);
        }

        // get starting log time 
        set_log_start();
        spi_write(temp_ts1, temp_ts2, current, voltage, bat_percentage, fault_name);

        // used to indicate 1st iteration of a state (terminal output preferences)
        disp_counter += 1;

    }

    else if (led_state == HIGH && disp_counter > 1){

        // Logging State: Green LED
        if (!BMS.FAULT_FLAG){
            analogWrite(LED_RED, 0);
            analogWrite(LED_GREEN, 255);
            analogWrite(LED_BLUE, 0);
        }      

        // write to .csv file, send to micro-sd card
        delay(1000);
        spi_write(temp_ts1, temp_ts2, current, voltage, bat_percentage, fault_name);
    }

    // momentary switch (LOW) --> idle state
    if (led_state == LOW)
        led_boot();

    if (disp_counter == 0)
        disp_counter++; // 2nd iteration of a state (will not print out repetitive message)


    // TODO:
    // if (led_state == HIGH && BMS.FAULT_FLAG){
    //     spi_write();
    // }

}


// ISR: momentary switch --> toggle led_state
void IRAM_ATTR toggle_logging(){
    if (!(BMS.FAULT_FLAG))
        led_state = !led_state;
        counter = 0;        // reset: new file 
        disp_counter = 0;
}

// switch_setup(): configures switch as an input_pullup, sets debounce time
void switch_setup(){
    Serial.println("switch_setup: Running...");
    Serial.println("-----------------------");
    pinMode(MOMENTARY_SWITCH, INPUT_PULLDOWN);
    button.setDebounceTime(50); // set debounce time to 50 milliseconds
    attachInterrupt(digitalPinToInterrupt(MOMENTARY_SWITCH), toggle_logging, FALLING);
}

#endif

