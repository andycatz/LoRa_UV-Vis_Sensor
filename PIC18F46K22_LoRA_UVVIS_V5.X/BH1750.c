#include <xc.h>
#include "i2c1.h"
#include "BH1750.h"

uint8_t bh1750Address=BH1750_ADDRESS_L; //Default

void setBH1750Address(uint8_t add){
    if(add>0){
        bh1750Address = BH1750_ADDRESS_H;
    }
    else{
        bh1750Address=BH1750_ADDRESS_L;
    }
}

void setBH1750ContinuousHResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_CONT_HRES_MODE);
}

void setBH1750ContinuousH2ResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_CONT_HRES2_MODE);
}

void setBH1750ContinuousLResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_CONT_LRES_MODE);
}

void setBH1750OneTimeHResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_ONETIME_HRES_MODE);
}

void setBH1750OneTimeH2ResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_ONETIME_HRES2_MODE);
}

void setBH1750OneTimeLResolutionMode(void){
    BH1750WriteCommand(bh1750Address, BH1750_ONETIME_LRES_MODE);
}

void bh1750ChangeMeasurementTime(uint8_t mTime){
    //Bits 7, 6, 5 are written using change high part
    uint8_t highPart = (mTime && 0b11100000) >> 4;
    highPart = highPart || BH1750_CHANGE_MEAS_TIME_H; //OR with command
    BH1750WriteCommand(bh1750Address, highPart);
    
    //Bits 4,3,2,1,0 are written using change low part
    uint8_t lowPart = (mTime && 0b00011111); //Extract low part
    lowPart = lowPart || BH1750_CHANGE_MEAS_TIME_L; //OR with command
    BH1750WriteCommand(bh1750Address, lowPart);
    //All done
}

/**
 * Write a command byte to the device
 * @param address   Slave address of device
 * @param command   Command to send
 */
void BH1750WriteCommand(uint8_t address, uint8_t command){
    I2C1_Start();
    I2C1_Write_Byte_Read_Ack(address);
    I2C1_Write_Byte_Read_Ack(command);
    I2C1_Stop();
}

/**
 * Reads the 16-bit value from the device
 * @return The light intensity value
 */
uint16_t BH1750ReadValue(){
    I2C1_Start();
    I2C1_Write_Byte_Read_Ack(bh1750Address+1u);
    char msb = I2C1_Read_Byte(1); //Read byte and send acknowledge
    char lsb = I2C1_Read_Byte(1); //Read byte and do not send acknowledge
    I2C1_Stop();
    return msb*256u+lsb; //Calculate the result;
}

