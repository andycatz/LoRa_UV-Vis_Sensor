# LoRa_UV-Vis_Sensor
by Andy Page, 6th April 2023
UV/Visible Light Sensor with LoRa transmitter 
Microcontroller: PIC18F46K22
Compiler: Microchip XC8 v2.31 (C90 C Standard)

Circuit description (see schematic SCH000038r1 included in the files).

The RFM95W LoRa module (868MHz) is connected to the PIC18F46K22 using SPI as follows:
PIC18F46K22                   RFM95W module
SCK2 (RD0) pin 38             pin 4
MISO2 (RD1) pin 39            pin 2
nSS2 (RD3) pin 41             pin 5
MOSI2 (RD4) pin 2             pin 3
None of the DIO pins or the RESET pin of the RFM95W module are connected.

UV Sensing is performed using a VEML6075 module (now unfortunately an obsolete device previously manufactured by Vishay, but you can still get them on ebay and from other suppliers)
This is connected to the PIC18F46K22 by I²C (SCL1, RC3, pin 37 and SDA1, RC4, pin 42).

Visible light sensing is performed using a BHV1750 module also connected to the same I²C bus as the VEML6075.
The power to both of these devices is controlled by PNP transistor Q1 (BC856).  This is turned on by the PIC by pulling RA2 (pin 21) to 0V.  Turning these devices off between measurements saves battery power.

All circuits are powered by two C alkaline cells giving a nominal 3V supply voltage.
The PIC measures the battery voltage through voltage divider R4/R13 feeding analogue input AN0 (Pin 19).  The PIC uses its internal 1.024V reference to do this measurement.  This divider is only powered up while doing a measurement.  It is controlled by the same transistor Q1.

The PIC measures local temperature through a divider made up of 10k NTC thermistor RT1 and R14 (10k).  This feeds analogue input AN1 (pin 20).  The PIC uses Vdd as a reference for this measurement.  This divider is only powered up while making measurements, using the same power switch, transistor Q1.

A serial port RX/TX is provided on J2 for debugging.  An RS232 level shifter can be connected externally for this purpose (power is provided on the header).

Programming is achieved through header J1, a standard Microchip programmer such as PICKIT3 can be used.

The PIC has a 16MHz crystal connected to the oscillator pins.  This is not strictly necessary since there is no super critical timing (except perhaps for the serial port when debugging).

LED1 (green LED) is turned on during transmitting of the data to give a visual indication of activity.

The PIC spends most of its time sleeping with all peripherals turned off.  It is woken approximately once per minute by a watchdog timer timeout.  On waking, it turns on the LED, turns on Q1, performs the necessary measurements of visible light, UV, battery voltage and local temperature.  Q1 is then turned off again.  The PIC then puts the data into a 50-byte buffer, calculates a 16-bit CRC of this messages and puts these bytes in the last two locations.  The LoRa RFM95W module is then woken up and initialised.  The 50-bytes of data are then transmitted.  The RFM95W module goes back to sleep automatically.  The PIC then turns off all internal peripherals and the LED and goes back to sleep, waiting for the next watchdog timer timeout.
Standby (sleeping) supply current from the batteries is about 42µA although in theory it should be lower than this.  However, even so, the batteries will last a good year or two.
Most of the "secret sauce" of setting up the RFM95W module is done in a single function called "LoRaOptimalLoad" in the file LoRa.C which just loads all the registers needed to send the data with the optimal values in one go.
No calibration is performed on any sensor data.  The bytes are transmitted exactly as read out of the sensors and A to D converters.  This minimises the size of the code and the time that the PIC is awake.  Calbration and calculation of the real values must be performed on the receiver which will likely have a permanent power supply and considerably more processing power.
This device does not support LoRaWAN, it justs uses LoRa as the method to transmit the data.

A complete version of this project can be found on andypageelectronics.wordpress.com
