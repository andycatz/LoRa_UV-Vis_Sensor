/*
 * File:   main.c
 * Author: andym
 * 
 * PIC18F46K22_LoRa_UVVIS_V2
 * 
 * Program to periodically transmit UV & visible light data using
 * LoRa transmitter on 868.0MHz. 866.5MHz now
 * 
 * Reference Sensor Types
 * 0    BASE RECEIVER
 * 1    Rain
 * 2    UV/Visible Light         <------------- THIS UNIT
 * 3    Temp/RH* (This sensor)
 * 4    Extra temp(s)
 * 5    Soil moisture
 * 6    Lightning       
 * 7    Radiation
 * 8    Wind
 * 9    Leaf wetness
 * 10   Electric Field
 * 11   External Pressure
 *
 * Created on 20 February 2021, 16:41
 * 
 * RA2 controls sensor & RS232 supply & divider & reference (turn off when not in use - high is off, low is on)
 * driven by PNP transistor from port pin to get enough voltage drive.

 * AN0 reads battery voltage through a resistor divider
 * AN1 reads local temperature through 10k NTC and 10k resistor as a divider from 3.3V
 * 
 * Uses BH1750 for visible light on i²c1
 * Uses VEML6075 for UV on i²c1
 * 
 * Version 2 5th March 2021:  Version 1 had current draw of 591µA when asleep!  Now 12.5µA
 *           2nd April 2021:  Modified to use Vdd for temperature reference and 1.024V internal reference for battery voltage.
 * 
 * Version 3 5th April 2021:  Modified message format, frequency and sync word to avoid LoRaWAN.
 *           15th April 2021: Added LoRa module reset function using RC6 microcontroller pin.
 *           16th April 2021: Removed 1k resistor between RC6 and radio module.  Reduced sleep current from 1.7mA (yes mA!) to 43µA.
 *           16th April 2021: Added code to turn off internal reference before sleep.  Sleep current now back to 12µA on new PCB!
 *           19th April 2021: Moved setupAtoD() to configIO() so that reference is up and stable by the time the A to D is used.
 * Version 4 16th May 2021:  Added UVLO to prevent transmitter operation with flat batteries.
 * Version 5 18th Sept 2021: New data format, 50 byte fixed packet length for all sensors.
 */          


#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pic18f46k22.h>
#include "config.h"
#include "usart2.h"
#include "LoRa.h"
#include "VEML6075.h"
#include "defines.h"
#include "i2c1.h"
#include "uv.h"
#include "BH1750.h"
#include "CRC16.h"

#define DEBUG 0
#define TX_FREQ 866.5
#define SYNC_WORD 0x55
#define BATT_UVLO 2100 //2V UVLO below which transmitter operation is prevented
#define BATT_UVLO_ATOD BATT_UVLO/4
#define DATA_PACKET_LENGTH 50
#define ID0 0x00
#define ID1 0x02
#define SOFTWARE_VERSION 0x05

void configureIO();
void readVisValue();
void transmitValues();
void turnStuffOff();
void disablePeripherals();
uint16_t readBattery();
uint16_t readTemperature();
void setupAtoD();

uint8_t txData[DATA_PACKET_LENGTH]; //Transmit buffer
uint8_t address[8] = {0x6E,0xDA,0x82,0x33,0x33,0x66,0xF5,0xE6}; //This should be unique

extern uint16_t uvaReading;
extern uint16_t uvbReading;
extern uint16_t comp1Reading;
extern uint16_t comp2Reading;
uint16_t vis=0;
uint16_t batt=0;
uint16_t temp=0;
uint32_t messageCount=0;

void main(void) {
    start:
    configureIO();  //Sets up all the required I/O pins to talk to stuff

    
    if(DEBUG){
        printf("LoRa868 UV/Visible Light Sensor\r\n");
    }
    setBH1750ContinuousHResolutionMode(); //Set visible light sensor to x1
    __delay_ms(180); //Need to wait for visible light sensor to take a measurement
    readUV(); //Reads the needed values from the UV sensor
    if(DEBUG){
        printf("UVA %d\r\n", uvaReading);
        printf("UVB %d\r\n", uvbReading);
        printf("CMP1 %d\r\n", comp1Reading);
        printf("CMP2 %d\r\n", comp2Reading);
    }
    readVisValue(); //Reads the value from the visible light sensor
    if(DEBUG){
        printf("VIS %d\r\n", vis);
    }
    
    batt = readBattery();
    temp = readTemperature();
    if(DEBUG){
        printf("BATT %d\r\n", batt);
        printf("TEMP %d\r\n", temp);
    }
    if(batt>BATT_UVLO_ATOD){
        transmitValues(); //Transmits the required bytes
    }
    else{
        //Flash the red LED 3 times to indicate flat battery
        RED_LED=1; //Red LED on
        __delay_ms(300);
        RED_LED=0;
        __delay_ms(300);
        RED_LED=1; //Red LED on
        __delay_ms(300);
        RED_LED=0;
        __delay_ms(300);
        RED_LED=1; //Red LED on
        __delay_ms(300);
        RED_LED=0;
        __delay_ms(300);
    }
    turnStuffOff(); //Turns everything off and prepares to sleep
    messageCount++;
    if(DEBUG){
        printf("Sleeping\r\n");
    }
    
    SLEEP(); //Go to sleep until the watchdog times out.
    //Resumes here after watchdog timeout
    //RESET(); //Reset after sleep - start over
    goto start;
}

void configureIO(){
    PMD2bits.ADCMD=0; //Turn ADC on
    if(DEBUG){
        PMD0bits.UART2MD=0; //Turn UART2 on
    }
    PMD1bits.MSSP1MD=0; //Turn I2C on
    PMD1bits.MSSP2MD=0; //Turn SPI2 on
    ANSELAbits.ANSA2=0; //Analogue off
    TRISAbits.RA2=0; //Output
    LATAbits.LATA2=0; //External circuitry on
    ANSELEbits.ANSE1=0; //Turn off analogue on RE1
    ANSELEbits.ANSE2=0; //Turn off analogue on RE2
    ANSELBbits.ANSB4=0; //Turn off analogue on RB4
    TRISEbits.RE1=0; //Green LED for status
    TRISEbits.RE2=0; //Red LED for status
    RED_LED=0; //Red LED off
    if(DEBUG){
        USART2_Start(BAUD_57600); //Start USART2
    }
    setupAtoD(); //Setup to read AN0 (reads supply voltage [battery])
    I2C1_Initialize(100000); //Starts I2C module 1 (100kHz fixed)
    I2C1_Check_Data_Stuck(); //Check if bus is stuck and attempt to unstick it.
    VEML6075Start();
    setBH1750Address(LOW); //Set address of BH1750 assuming ADDR pin is pulled low
    setBH1750ContinuousHResolutionMode(); //Set the device mode
}

void readVisValue(){
    vis = BH1750ReadValue();
}



/**
 * Transmits the sensor values as a sequence of bytes
 */
void transmitValues(){
    txData[0] = DATA_PACKET_LENGTH;
    txData[1] = ID0; //Copy in the ID
    txData[2] = ID1; //Copy in the ID
    for(uint8_t i=0;i<8;i++){
        txData[i+3] = address[i]; //Copy in the address
    }
    txData[11] = SOFTWARE_VERSION;
    
    //Message count
    txData[12]=(uint8_t)((messageCount>>24)&0xFF); //MSB
    txData[13]=(uint8_t)((messageCount>>16)&0xFF); //Upper middle
    txData[14]=(uint8_t)((messageCount>>8)&0xFF); //Lower middle
    txData[15]=(uint8_t)((messageCount & 0xFF)); //LSB
    
    //Supply voltage value (10-bit in 2 bytes)
    txData[16]=(uint8_t)((batt>>8)&0xFF); //MSB
    txData[17]=(uint8_t)(batt & 0xFF); //LSB
    
    //Sensor local temperature value (16-bit)
    txData[18]=(uint8_t)((temp>>8)&0xFF); //MSB
    txData[19]=(uint8_t)(temp & 0xFF); //LSB
    
    //V1 Voltage (0)
    txData[20] = 0;
    txData[21] = 0;
    
    //V2 Voltage (0)
    txData[22] = 0;
    txData[23] = 0;
    
    //UVA sensor value
    txData[24]=(uint8_t)((uvaReading>>8)&0xFF); //MSB
    txData[25]=(uint8_t)(uvaReading & 0xFF); //LSB
    
    //UVB sensor value
    txData[26]=(uint8_t)((uvbReading>>8)&0xFF); //MSB
    txData[27]=(uint8_t)(uvbReading & 0xFF); //LSB
    
    //COMP1 value
    txData[28]=(uint8_t)((comp1Reading>>8)&0xFF); //MSB
    txData[29]=(uint8_t)(comp1Reading & 0xFF); //LSB
    
    //COMP2 value
    txData[30]=(uint8_t)((comp2Reading>>8)&0xFF); //MSB
    txData[31]=(uint8_t)(comp2Reading & 0xFF); //LSB
    
    //Visible light value
    txData[32]=(uint8_t)((vis>>8)&0xFF); //MSB
    txData[33]=(uint8_t)(vis & 0xFF); //LSB
    
    //Fill the rest of the data area with 0
    for(uint8_t i=34;i<48;i++){
        txData[i] = 0;
    }
    
    //Calculate CRC16 and add to end of message
    unsigned short int calcCRC = CRC16(txData, DATA_PACKET_LENGTH-2);
    txData[49] = (calcCRC&0xFF00u)>>8u; //MSB
    txData[48] = (calcCRC&0xFF); //LSB
    
    
    if(DEBUG){
        printf("Starting transmitter...\r\n");
    }
    LoRaStart(TX_FREQ, SYNC_WORD); //Configure module
    if(DEBUG){
        printf("TXF: %f\r\n", LoRaGetFrequency());
    }
    LoRaClearIRQFlags();
    RED_LED=1; //Red LED on (saves battery power by doing it here!)
    LoRaTXData(txData, DATA_PACKET_LENGTH); //Send data
    RED_LED=0; //Red LED off (saves battery power by doing it here!)
    if(DEBUG){
        printf("Wait for end of transmission...\r\n");
    }
    uint8_t j=0;
    for(j=0;j<50;j++){
        uint8_t flags = LoRaGetIRQFlags();
        //printf("IRQ %d %d \r\n",j, flags);
        if(flags>0){
            break;
        }
        __delay_ms(10); //We are done with transmission
    }
    if(DEBUG){
        if(j>48){
            printf("TX Fail\r\n");
        }
        else{
            printf("Done.\r\n");
        }
    }
    LoRaSleepMode(); //Put module to sleep
    __delay_ms(10);
}

void turnStuffOff(){
    USART2_Stop();
    disablePeripherals();
}

void disablePeripherals(){
    ADCON0bits.ADON=0; //Turn off A to D module
    VREFCON0bits.FVREN=0; //Disable internal reference
    //Set all pins as outputs
    TRISA=0;
    TRISB=0;
    TRISC=0;
    TRISD=0;
    TRISE=0;
    LATA=0;
    LATB=0;
    LATC=0;
    LATD=0;
    LATE=0;
    TRISCbits.RC6=1; //Configure port as input (goes high-Z) - this is LoRa module reset line
    LATCbits.LATC6=1;
    TRISCbits.RC3=1; //I2C Clock line
    TRISCbits.RC4=1; //I2C data line
    LATAbits.LA2=1; //Turn off external peripherals
    TRISDbits.RD7=1; //RX on UART2
    //Deal with SPI in case LoRa module is driving it
    TRISDbits.RD1=1; //SDIx must have corresponding TRIS bit set (input)
    ANSELDbits.ANSD1=0; //Input buffer enabled
    LATDbits.LATD3=1; //Set SS high so LoRa chip is not selected
//    PMD0bits.UART2MD=1; //Turn off UART2
//    PMD0bits.UART1MD=1; //Turn off UART1
//    PMD0bits.TMR6MD=1; //Turn off timer 6
//    PMD0bits.TMR5MD=1; //Turn off timer 5
//    PMD0bits.TMR4MD=1; //Turn off timer 4
//    PMD0bits.TMR3MD=1; //Turn off timer 3
//    PMD0bits.TMR2MD=1; //Turn off timer 2
//    PMD0bits.TMR1MD=1; //Turn off timer 1
//    PMD0bits.SPI2MD=1; //Turn off SPI2
//    PMD0bits.SPI1MD=1; //Turn off SPI1
    PMD0=0xFF; //Turn off all peripherals in PMD0 (UARTS, SPI)
    PMD1=0xFF; //Turn off all peripherals in PMD1
    PMD2=0xFF; //Turn off all peripherals in PMD2 (ADC, comparators, CTMU)
}

/**
 * Reads the supply voltage A to D
 */
uint16_t readBattery(){
    ADCON1bits.PVCFG=0b10; //A/D Vref+ connected to internal reference FVR BUF2
    ADCON0bits.GO_NOT_DONE=1; //Start the A to D process
    while(ADCON0bits.GO_NOT_DONE){
        //Wait for conversion to complete (about 15µs)
    }
    uint16_t result = ADRESH * 256 + ADRESL; //Read A to D result
    return result;
}

uint16_t readTemperature(){
    ADCON1bits.PVCFG=0; //A/D Vref+ connected to Vdd
    //Select channel 1 for A to D
    ADCON0bits.CHS=1;
    ADCON0bits.GO_NOT_DONE=1; //Start the A to D process
    while(ADCON0bits.GO_NOT_DONE){
        //Wait for conversion to complete (about 15µs)
    }
    uint16_t result = ADRESH * 256 + ADRESL; //Read A to D result
    return result;
}

void setupAtoD(){
    //Setup AN0 for a to d converter (RA0)
    
    //Set ANSELbit to disable digital input buffer
    ANSELAbits.ANSA0=1;
    ANSELAbits.ANSA1=1;
    
    //Set TRISXbit to disable digital output driver
    TRISAbits.RA0=1;
    TRISAbits.RA1=1;
    
    //Set voltage references
    ADCON1bits.PVCFG=0; //A/D Vref+ connected to Vdd
    ADCON1bits.NVCFG=0; //A/D Vref- connected to internal signal AVss
    VREFCON0bits.FVRS=0b01; //Fixed voltage reference is 1.024V
    VREFCON0bits.FVREN=1; //Enable internal reference
    
    //Select channel 0 for A to D
    ADCON0bits.CHS=0;
    
    //Set A to D acquisition time
    ADCON2bits.ACQT=0b010; //Tacq = 4 Tad (4µs)
    
    //Set A to D clock period
    ADCON2bits.ADCS=0b110; //Clock period set to Fosc/64 = 1µs (64MHz clock)
    
    //Set result format
    ADCON2bits.ADFM = 1; //Data is mostly in the ADRESL register with 2 bits in the ADRESH register
    
    //Turn on the A to D module
    ADCON0bits.ADON=1;
}
