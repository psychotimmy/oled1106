# oled1106
Display library (I2C) for 128x64 (132x64) oled using a SH1106 driver.

Pre-requisites: A Raspberry Pi running Raspbian Buster with the pigpiod library and daemon installed and running.

This library was written to work with a 1.3" oled display from https://www.az-delivery.de/en/products/1-3zoll-i2c-oled-display?_pos=1&_sid=57636400d&_ss=r

It should work with any similar I2C display.

Two test programs are provided:

oled1106test - write some sample text and graphics to the display

oled1106life - John Conway's life game, adapted for this display

The code is reasonably well documented, if sub-optimal in places.

Tim Holyoake, 9th May 2020.
