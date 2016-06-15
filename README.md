# awTimer
* Small battery powered device with nanoWatt Technology from Microchip.
* devise start count seconds when light is switched on, first 3 sec indicated by LED blinks
* when light goes off in case if seconds are more then 240 motor activated for 700ms after 5 blinks of LED 
* motor can be activated manually by button press whel light is on
 

## HW/SW
- PIC12F683
- MPLAB® X IDE v3.30 + MPLAB® XC8 Compiler v1.37

### Pin connections
- GP0 pin7 NC
- GP1 pin6 NC
- GP2 pin5 light sensor to Vdd + 1MOhm to Vss
- GP3 pin4 tactile switch to Vdd + 1MOhm to Vss
- GP4 pin3 LED -> resistor to the Vdd
- GP5 pin2 to transisor -> motor + protection diode

# Enjoy !