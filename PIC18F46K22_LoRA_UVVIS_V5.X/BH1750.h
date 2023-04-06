// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef BH1750_H
#define	BH1750_H

//Addresses for device depending on whether ADDR pin is set high or low
#define BH1750_ADDRESS_H 0b10111000
#define BH1750_ADDRESS_L 0b01000110
#define BH1750_CONT_HRES_MODE 0b00010000
#define BH1750_CONT_HRES2_MODE 0b00010001
#define BH1750_CONT_LRES_MODE 0b00010011
#define BH1750_ONETIME_HRES_MODE 0b00100000
#define BH1750_ONETIME_HRES2_MODE 0b00100001
#define BH1750_ONETIME_LRES_MODE 0b00100011
#define BH1750_CHANGE_MEAS_TIME_H 0b01000000
#define BH1750_CHANGE_MEAS_TIME_L 0b01100000
#define LOW 0
#define HIGH 1

#include <xc.h> // include processor files - each processor file is guarded. 
#include "i2c1.h" //I2c library
#include <stdint.h>

/**
 * Writes the continuous reading HResolution mode to the device
 * @param 
 */
void setBH1750Address(uint8_t);
void setBH1750ContinuousHResolutionMode(void);
void setBH1750ContinuousH2ResolutionMode(void);
void setBH1750ContinuousLResolutionMode(void);
void setBH1750OneTimeHResolutionMode(void);
void setBH1750OneTimeH2ResolutionMode(void);
void setBH1750OneTimeLResolutionMode(void);
void bh1750ChangeMeasurementTime(uint8_t);
void BH1750WriteCommand(uint8_t, uint8_t);
uint16_t BH1750ReadValue();


#endif	/* BH1750_H */
