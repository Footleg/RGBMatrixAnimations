# Example Programs for RGB LED Matrix devices
This folder contains a set of example programs using the LED animaton classes with an RGB LED Matrix. A Makefile is provided to compile these for the RGB Matrix library by Henner Zeller to run on a Raspberry Pi. First check out the https://github.com/hzeller/rpi-rgb-led-matrix library onto a Raspberry Pi. This can be installed following the instructions for the Adafruit RGB Matrix HAT https://learn.adafruit.com/adafruit-rgb-matrix-plus-real-time-clock-hat-for-raspberry-pi/driving-matrices 
Once you have the hzeller/rpi-rgb-led-matrix library deployed on your Raspberry Pi, build the rgbmatrix library and run the examples provided with that libray first. This done from a command prompt in the rpi-rgb-led-matrix folder, run the command: 
```bash
HARDWARE_DESC=adafruit-hat make
```
Run the first demo using the following command for a Raspberry Pi 4 (see the rpi-rgb-led-matrix library docs for slower Pi versions):
```bash
sudo ./demo --led-slowdown-gpio=4 -D0
```
Now check out this RGBMatrixAnimations repo into the rpi-rgb-led-matrix folder. Then enter the RGBMatrixAnimations/examples/RGBMatrix_RPi folder and build the example animations programs. 
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
The other examples also explain their own command line options (if you enter -h, or any options that are invalid). Try the Game of Life:
```bash
sudo ./gol --led-slowdown-gpio=4
```
Try Falling Sand:
```bash
sudo ./sand --led-slowdown-gpio=4 -n 200 -g 10 -s 5
```