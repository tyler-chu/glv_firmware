#ifndef BMS_CONFIG
#define BMS_CONFIG

#include <Arduino.h>
#include <Wire.h>
#include <bq769x0.h>
#include <registers.h>
#include <chrono>
#include <LED_Config.h>

#define I2C_SDA 7
#define I2C_SCL 9
#define ALERT_PIN 17

// const int SLAVE_ADDRESS = 0x18;                  // model 7: ic
const int SLAVE_ADDRESS = 0x08;                     // model 3: ic
bq769x0 BMS;                                        // BMS Object 
float temp, temp_ts1, temp_ts2, current, voltage;     // used for BMS
hw_timer_t *status_cfg = NULL;

// void IRAM_ATTR status_ISR(){
//     if (!FAULT_FLAG)
//         BMS.checkStatus();
// }

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
    BMS.updateVoltages();

    BMS.setThermistorBetaValue(3977);
    BMS.setTemperatureLimits(-20, 45, 0, 45);
    BMS.setShuntResistorValue(5);
    BMS.setShortCircuitProtection(14000, 200);
    // BMS.setOvercurrentChargeProtection(8000, 200);
    BMS.setOvercurrentDischargeProtection(8000, 320);
    BMS.setCellUndervoltageProtection(2600, 2);
    BMS.setCellOvervoltageProtection(3650, 2);

    BMS.setBalancingThresholds(0, 3300, 20);
    BMS.setIdleCurrentThreshold(100);
}

// bms_setup: configures BMS for data reading/protection
void bms_setup(){
    Serial.println("bms_setup(): Running...");
    BMS.begin(ALERT_PIN, 3);   // (alert, boot)

    // config timer for interrupt (5 sec timer)
    // status_cfg = timerBegin(0, 40, true);
    // timerAttachInterrupt(status_cfg, &status_ISR, true);
    // timerAlarmWrite(status_cfg, 5000000, true);
    // timerAlarmEnable(status_cfg);
    
    bms_set_protection();
    BMS.enableAutoBalancing();
    BMS.enableCharging();
    BMS.enableDischarging();
    
    Serial.println("-----------------------");

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

    Serial.println("-----------------------");

}

/**
 * @brief i2c_rw_test(): read/write to SYS_CTRL1 register
 * - read 24 from SYS_CTRL1
 * - write 0 to SYS_CTRL1 & read 0
 * - reset SYS_CTRL1 back to 24
 */
void i2c_rw_test(){
    Serial.println("i2c_rw_test(): Running...");

    // read (0x18) 24 from SYS_CTRL1
    Serial.print("- Read [Dec]: (");
    Serial.print(BMS.readRegister(SYS_CTRL1));
    Serial.println(")\t <-- SYS_CTRL1 Register");

    // write (0x00) 0 to SYS_CTRL1
    Serial.println("- Write [Hex]: (0x00)\t --> SYS_CTRL1 Register");
    BMS.writeRegister(SYS_CTRL1, 0x00);

    // read (0x00) 0 from SYS_CTRL1
    Serial.print("- Read [Dec]: (");
    Serial.print(BMS.readRegister(SYS_CTRL1));
    Serial.println(")\t <-- SYS_CTRL1 Register");

    // write (0x18) 24 to SYS_CTRL1
    Serial.println("- Write [Hex]: (0x18)\t --> SYS_CTRl1 Register");
    BMS.writeRegister(SYS_CTRL1, 24);

    // read (0x18) 24 from SYS_CTRl1
    Serial.print("- Read [Dec]: (");
    Serial.print(BMS.readRegister(SYS_CTRL1));
    Serial.println(")\t <-- SYS_CTRL1 Register");

    Serial.println("-----------------------");
}

// get_bms_values(): get temp, current, voltage readings
void get_bms_values(){
    // Serial.println("get_bms_values(): Running ...");

    BMS.updateBalancingSwitches();

    BMS.updateTemperatures();
    temp_ts1 = BMS.getTemperatureDegC(TS1_CHANNEL);
    BMS.updateTemperatures2();
    temp_ts2 = BMS.getTemperatureDegC(TS2_CHANNEL);
    // BMS.updateCurrent();
    BMS.updateVoltages();

    current = BMS.getBatteryCurrent();

    float divider = 1000.00f;
    voltage = (BMS.getBatteryVoltage())/divider;

    Serial.print("Temp TS1: ");
    Serial.println(temp_ts1);

    Serial.print("Temp TS2: ");
    Serial.println(temp_ts2);

    Serial.print("I_o: ");
    Serial.println(current);

    Serial.print("V_bat: ");
    Serial.println(voltage);

    Serial.println("-----------------------");

    delay(1000);

}

// bms_loop(): recieves bms values, displays data, checks status of BM IC
//             loops through logging processes
void bms_loop(){
    Serial.println();
}

#endif

