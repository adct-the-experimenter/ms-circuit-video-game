
# Switched Tag Render Module code

This is the personal computer side code that takes in digitized information on 
analog signals and interprets it into an image.

This is done with a USB UART bridge present on a stm32 microcontroller - or other microcontroller- 
that is used to communicate between microcontroller and PC

This specific render code is for switched tag.

## ADC UART Data Transfer Firmware
The firmware for read ADC values and transfering those values to PC over UART 
is located in console-hardware/render-module-hardware/uController-uart-pc-firmware.
