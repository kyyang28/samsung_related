#!/bin/sh

# By using mktiny210spl.exe tool, we can add the header info for the leds_sd_tiny210_semi.bin
# in order to run the program properly on the board
./mktiny210spl.exe keys_leds_sd_tiny210_semi.bin keys_leds_sd_tiny210.bin

# Adding the execution authority of the leds_sd_tiny210.bin
chmod +x keys_leds_sd_tiny210.bin

