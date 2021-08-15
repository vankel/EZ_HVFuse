# EZ_HVFuse
AVR Fuse Rescue for ATmega48/88/168/328 based on Arduino-based AVR High Voltage Programmer by Jeff Keyzer W6OHM aka @ mightyohm.

## Schematics
![AVR Fuse Rescue Schematics](https://raw.githubusercontent.com/vankel/EZ_HVFuse/master/schematics/EZ_HVFuse_Sch.png)

## How To Use
1. Connect the bricked microcontroller to your Arduino board.
2. Input 12V Power.
3. Upload the sketch to your Arduino board. 
4. Use the Serial Monitor at 9600 Baud.
5. Send "D" or "d" for Detect the chip.
6. Send "C" or "c" to erase chip.
7. Send "W" or "w" to write default fuse value.
