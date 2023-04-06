#ifndef VEML6075_H
#define	VEML6075_H

#include <xc.h> // include processor files - each processor file is guarded. 

//Registers in the VEML6075
#define VEML6075_UV_CONF_REG 0x00
#define VEML6075_UVA_REG 0x07
#define VEML6075_UVB_REG 0x09
#define VEML6075_UV_COMP1_REG 0x0A
#define VEML6075_UV_COMP2_REG 0x0B
#define VEML6075_ID_REG 0x0C
#define VEML6075_SLAVE_ADDRESS 0x20
#define VEML6075_ConfigLSB 0
#define VEML6075_ConfigMSB 0
#define INT_TIME  100

/**
 * Writes a configuration word to the VEML6075 device.
 * @param  Device hardware address.
 * @param  Command code
 * @param  Data byte low
 * @param  Data byte high
 */
void writeVEML6075(uint8_t, uint8_t, uint8_t, uint8_t);

/**
 * Reads a 16-bit unsigned value from the specified VEML6075 device
 * @param  The device hardware address.
 * @param  Command code
 * @return The 16-bit signed value read from the device.
 */
uint16_t readVEML6075(uint8_t, uint8_t);

void VEML6075Start();

#endif	/* XC_HEADER_TEMPLATE_H */

