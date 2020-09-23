# Animations classes for RGB LED Matrix devices
This folder contains a set of animator classes to generate LED array animations. These are reusable with any LED matrix or other display device which supports RGB colour values per pixel or LED. A Makefile is provided to compile these for the RGB Matrix library by Henner Zeller to run on a Raspberry Pi. First check out the https://github.com/hzeller/rpi-rgb-led-matrix library onto a Raspberry Pi. This can be installed following the instructions for the Adafruit RGB Matrix HAT https://learn.adafruit.com/adafruit-rgb-matrix-plus-real-time-clock-hat-for-raspberry-pi/driving-matrices 
Once you have the hzeller/rpi-rgb-led-matrix library deployed on your Raspberry Pi, copy this animations folder into the rpi-rgb-led-matrix folder. You will need to build the rgbmatrix library first, from a command prompt in the rpi-rgb-led-matrix folder, run the command: 
```bash
HARDWARE_DESC=adafruit-hat make
```
Then enter the animations folder and build example animations programs. 
```bash
make
```
Run the simplecrawler example with the command line (suitable for a Raspberry Pi 4):
```bash
sudo ./simplecrawler --led-slowdown-gpio=4 -m 30
```
The command line options are explained under the demo project in rpi-rgb-led-matrix/examples-api-use, or run the example with this command for a summary of command line options:
```bash
sudo ./simplecrawler -h
```