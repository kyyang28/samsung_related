#!/bin/sh

# By using mktiny210spl.exe tool, we can add the header info for the leds_sd_tiny210_semi.bin
# in order to run the program properly on the board
./mktiny210spl.exe int_ddr2_sd_tiny210_semi.bin int_ddr2_sd_tiny210.bin

# Adding the execution authority of the leds_sd_tiny210.bin
chmod +x int_ddr2_sd_tiny210.bin

