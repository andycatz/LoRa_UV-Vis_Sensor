#include <xc.h>
#include <stdint.h>
#include "defines.h"
#include "VEML6075.h"
#include "i2c1.h"

uint16_t uvaReading=0;
uint16_t uvbReading=0;
uint16_t comp1Reading=0;
uint16_t comp2Reading=0;

void readUV(void){
    I2C1_Check_Data_Stuck(); //This can happen if there is a lot of EMI on the sensor
    uvaReading = readVEML6075(VEML6075_SLAVE_ADDRESS, VEML6075_UVA_REG);
    uvbReading = readVEML6075(VEML6075_SLAVE_ADDRESS, VEML6075_UVB_REG);
    comp1Reading = readVEML6075(VEML6075_SLAVE_ADDRESS, VEML6075_UV_COMP1_REG);
    comp2Reading = readVEML6075(VEML6075_SLAVE_ADDRESS, VEML6075_UV_COMP2_REG);  
}
