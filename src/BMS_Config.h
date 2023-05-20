#ifndef BMS_CONFIG
#define BMS_CONFIG

#include <Arduino.h>
#include <Wire.h>
#include <bq769x0.h>
#include <registers.h>
#include <chrono>
#include <LED_Config.h>
#include <esp_sleep.h>

#define I2C_SDA 7
#define I2C_SCL 9
#define ALERT_PIN 5

// const int SLAVE_ADDRESS = 0x18;                  // model 7: ic
const int SLAVE_ADDRESS = 0x08;                     // model 3: ic
bq769x0 BMS;                                        // BMS Object 
float temp, temp_ts1, temp_ts2, current, voltage, bat_percentage;     // used for BMS
hw_timer_t *status_cfg = NULL;

// i2c_setup: sets up communication between BM IC (PCB) & ESP-32
void i2c_setup(){
    Wire.begin(I2C_SDA, I2C_SCL);
    // Serial.begin(115200);
    delay(1000);
    Serial.println("i2c_setup(): Running...");
}

// i2c_scanner: scan for slave address 
void i2c_scanner(){

    bool address_flag = false;

    while (!address_flag){
        byte error, address;
        int nDevices;
        Serial.println("Scanning...");
        nDevices = 0;

        // scans through 128 different addresses to locate the address of the slave (in HEX)
        for(address = 1; address < 127; address++ ) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            
            // error = 0: success
            if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address<16) {
                Serial.print("0");
            }
            Serial.println(address,HEX);
            nDevices++;

            address_flag = true;
            }

            // error = 4: other error 
            else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address<16) {
                Serial.print("0");
            }
            Serial.println(address,HEX);
            }    
        }

        if (nDevices == 0) {
            Serial.println("No I2C devices found\n");
        }
        else {
            Serial.println("done\n");
        }
        delay(5000);          
    }

}

// bms_set_protection: sets up thresholds for protection
void bms_set_protection(){
    Serial.println("bms_set_protection(): Running...");

    BMS.updateTemperatures();
    // BMS.updateTemperatures2();
    BMS.updateVoltages();

    BMS.setThermistorBetaValue(3977);
    BMS.setTemperatureLimits(-350, 550, 0, 550);
    // BMS.setTemperatureLimits(-35, 80, 0, 80);
    BMS.setShuntResistorValue(5);

    // comment this out for SCD test
    BMS.setShortCircuitProtection(18000, 200);
    BMS.setOvercurrentChargeProtection(18000, 200);
    BMS.setOvercurrentDischargeProtection(18000, 320);

    // short circuit detection test
    // BMS.setShortCircuitProtection(3000, 200);

    BMS.setCellUndervoltageProtection(2800, 2);
    BMS.setCellOvervoltageProtection(4150, 2);

    BMS.setBalancingThresholds(0, 3600, 20);    // later on change to 10 mV
    BMS.setIdleCurrentThreshold(100);

}

// bms_setup: configures BMS for data reading/protection
void bms_setup(){
    Serial.println("bms_setup(): Running...");
    BMS.begin(ALERT_PIN, 3);   // (alert, boot)
    
    bms_set_protection();
    BMS.enableAutoBalancing();
    BMS.enableCharging();
    BMS.enableDischarging();
    
    Serial.println("-----------------------------");

}

// clear_sys_stat: clear SYS_STAT register (controls enabling of temp, current, voltage readings, xready bit)
void clear_sys_stat(){
    Serial.println("clear_sys_stat(): Running...");

    // read from SYS_STAT register
    Serial.print("- Read [Dec]: (");
    Serial.print(BMS.readRegister(SYS_STAT));
    Serial.println(")\t <-- SYS_STAT Register");

    // write (0xFF) to SYS_STAT register
    Serial.println("- Write [Hex]: (0xFF)\t --> SYS_STAT Register");
    BMS.writeRegister(SYS_STAT, 0xFF);  // clearing SYS_STAT by writing 1's to the bits

    // read from SYS_STAT register
    Serial.print("- Read [Dec]: (");
    Serial.print(BMS.readRegister(SYS_STAT));
    Serial.println(")\t <-- SYS_STAT Register");

    Serial.println("-----------------------------");

}

void checkSCD() {
    if (BMS.SCD_FLAG == true) {
        Serial.println("- BMS.shutdown(): Running...");
        BMS.shutdown();
        Serial.println("- ESP-32: Entering Deep-Sleep Mode");
        esp_deep_sleep_start();
    }
}

// get_bms_values(): get temp, current, voltage readings
void get_bms_values(){
    // Serial.println("get_bms_values(): Running ...");

    // if (BMS.FAULT_FLAG == true)
    //     return;

    regSYS_STAT_t system_status;
    system_status.regByte = BMS.readRegister(SYS_STAT);
    Serial.print("SYS_STAT: ");
    Serial.println(system_status.regByte);
    
    BMS.fault_counter = 0;

    // BMS.checkStatus();

    BMS.updateBalancingSwitches();

    BMS.updateTemperatures();
    temp_ts1 = BMS.getTemperatureDegC(TS1_CHANNEL);
    BMS.updateTemperatures2();
    temp_ts2 = BMS.getTemperatureDegC(TS2_CHANNEL);

    //manually switch on CC_EN
    // BMS.writeRegister(SYS_CTRL2, B01000000);  // switch CC_EN on

    // BMS.updateCurrent(true);
    BMS.updateCurrent();
    BMS.updateVoltages();

    float divider = 1000.00f;

    current = (BMS.getBatteryCurrent())/divider;
    voltage = (BMS.getBatteryVoltage())/divider;

    // Serial.print("Temp TS1 [Ext/Ambient]: ");
    Serial.print("Temp TS1: ");
    Serial.println(temp_ts1);

    // Serial.print("Temp TS2 [Int/Die]: \t");
    Serial.print("Temp TS2: ");
    Serial.println(temp_ts2);
    
    Serial.print("I_o: \t  ");
    Serial.println(current);

    // Serial.print("V_bat [Pack Voltage]: \t");
    Serial.print("V_bat:    ");
    Serial.println(voltage);

    // max voltage: (4.15 * 6 = 24.9)
    // min voltage: (2.8 * 6 = 16.8)
    // range = 8.1 v

    // bat_percentage = ((voltage - 16.8) / 8.1) * 100;
    bat_percentage = BMS.get_percentage();

    Serial.print("Bat %:    ");
    Serial.println(bat_percentage);

    // SYS_CTRL2
    regSYS_STAT_t system_status2;
    system_status2.regByte = BMS.readRegister(SYS_CTRL2);
    Serial.print("SYS_CTRL2: ");
    Serial.println(system_status2.regByte);

    Serial.println("-----------------------------");

    delay(1000);


    // OV/UV Check

    if (bat_percentage > 100)
        BMS.OV_FLAG= true;
        // throw OV fault flag
    if (bat_percentage < 0)
        BMS.UV_FLAG = true;
        // throw UV fault flag
}

#endif

