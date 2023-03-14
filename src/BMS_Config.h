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

const int SLAVE_ADDRESS = 0x18;

bq769x0 BMS;    // BMS Object 

float temp, current, voltage;   // used for BMS

// i2c_setup: sets up communication between BM IC (PCB) & ESP-32
void i2c_setup(){
    Wire.begin(I2C_SDA, I2C_SCL);
    // Serial.begin(115200);
    delay(1000);
    Serial.println("i2c_setup(): Running...");
}

// bms_set_protection: sets up thresholds for protection
void bms_set_protection(){
    Serial.println("bms_set_protection(): Running...");
    BMS.setTemperatureLimits(0, 0, 0, 0);
    BMS.setShortCircuitProtection(0);
    BMS.setOvercurrentChargeProtection(0);
    BMS.setOvercurrentDischargeProtection(0);
    BMS.setCellUndervoltageProtection(0);
    BMS.setCellOvervoltageProtection(0);
}

// bms_setup: configures BMS for data reading/protection
void bms_setup(){
    Serial.println("bms_setup(): Running...");
    BMS.begin(ALERT_PIN, 3);   // (alert, boot)
    
    // BMS.enableCharging();
    // BMS.enableDischarging();
    // BMS.updateBalancingSwitches();
    
    // bms_set_protection();
    
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

    // BMS.setThermistorBetaValue(1);
    BMS.updateTemperatures();
    BMS.updateCurrent();
    BMS.updateVoltages();
    
    temp = BMS.getTemperatureDegC();
    current = BMS.getBatteryCurrent();
    voltage = BMS.getBatteryVoltage();

    // Serial.print("Temp: ");
    // Serial.println(temp);

    // Serial.print("I_o: ");
    // Serial.println(current);

    // Serial.print("V_bat: ");
    // Serial.println(voltage);

    // Serial.println("-----------------------");

    delay(1000);

}

// bms_loop(): recieves bms values, displays data, checks status of BM IC
//             loops through logging processes
void bms_loop(){
    Serial.println();
}

#endif

