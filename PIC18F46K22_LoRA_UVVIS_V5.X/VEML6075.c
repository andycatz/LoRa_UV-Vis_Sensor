/**
 * VEML6075.c
 * Functions to read and write the VEML6075 UV Sensor I.C.
 */

#include <xc.h>
#include "i2c1.h"
#include "defines.h"
#include <stdint.h>
#include "VEML6075.h"

uint8_t i2cFault=0;




/**
 * Writes the configuration bytes to the device
 * @param address
 * @param config
 */
void writeVEML6075(uint8_t address, uint8_t command, uint8_t dataByteLow, uint8_t dataByteHigh){
    I2C1_Start();
    i2cFault = I2C1_Write_Byte_Read_Ack(address);
    i2cFault = I2C1_Write_Byte_Read_Ack(command);
    i2cFault = I2C1_Write_Byte_Read_Ack(dataByteLow);
    i2cFault = I2C1_Write_Byte_Read_Ack(dataByteHigh);
    I2C1_Stop();
}

/**
 * Reads the 16-bit value from the device
 * @param address   Hardware I²C address
 * @return          16-bit signed data from device
 */
uint16_t readVEML6075(uint8_t address, uint8_t command){
    I2C1_Start();
    i2cFault = I2C1_Write_Byte_Read_Ack(address); //Write
    i2cFault = I2C1_Write_Byte_Read_Ack(command);
    I2C1_Repeated_Start(); //Send repeated start
    //Send repeated start condition
    i2cFault = I2C1_Write_Byte_Read_Ack(address+1u); //Read
    char lsb = I2C1_Read_Byte(1); //Read byte and send acknowledge
    char msb = I2C1_Read_Byte(0); //Read byte and do not send acknowledge
    I2C1_Stop();
    return msb*256u+lsb; //Calculate the result;
}

/*
 Sets the VEML6075 into the correct state.
 */
void VEML6075Start(){
    uint8_t configLSB = 0;
    if(INT_TIME==100){
        configLSB = configLSB | 0b00010000; //Set bit 4 high
    }
    else if(INT_TIME==200){
        configLSB = configLSB | 0b00100000; //Set bit 5 high
    }
    else if(INT_TIME==400){
        configLSB = configLSB | 0b00110000; //Set bits 4 & 5 high
    }
    else if(INT_TIME==800){
        configLSB = configLSB | 0b01000000; //Set bit 6 high
    }
    if(UV_HR==1){
        configLSB = configLSB | 0b00001000; //Set bit 3 high
    }
    writeVEML6075(VEML6075_SLAVE_ADDRESS, VEML6075_UV_CONF_REG, configLSB, VEML6075_ConfigMSB);
}

