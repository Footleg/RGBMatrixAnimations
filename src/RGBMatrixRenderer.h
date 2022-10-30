/**************************************************************************************************
 * RGB Matrix Renderer abstract class 
 * 
 * This abstract class provides a template for building renderer classes to control LED Matrix
 * displays. It provides methods to set the colour of any LED/pixel in a grid, and to keep the
 * position of points within the bounds of the grid (with or without wrapping over the edges).
 * A series of animation classes use the interface defined in this class to output to displays.
 * By writing a renderer class for any LED matrix hardware (or on screen display) this allows the
 * animations classes to be used with many different hardware display devices without requiring
 * code changes in the animation class.
 * 
 * This class also implements a wrapping mode for panels arranged as faces of a cube, including
 * tracking the x and y components of acceleration in the plane of each face of the cube for a
 * 3D acceleration vector. The enables animations to update the motion of pixels on any panel 
 * based on the cube face they appear on, and wrapping of pixels across panel edges in cube mode.
 *
 * Copyright (C) 2022 Paul Fretwell - aka 'Footleg'
 * 
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(ARDUINO)
#include "WProgram.h"
#else
#include <stdint.h>
#include <stdio.h>
#include <cmath>
#endif

struct RGB_colour {
    RGB_colour() : r(0), g(0), b(0) {}
    RGB_colour(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct MovingPixel {
    MovingPixel(uint16_t posX, uint16_t posY, int8_t velX, int8_t velY) : x(posX), y(posY), fineX(0), fineY(0),vx(velX), vy(velY) {}
    uint16_t x;
    uint16_t y;
    int16_t fineX;
    int16_t fineY;
    int8_t vx;
    int8_t vy;
};

class RGBMatrixRenderer
{
    //variables
    public:
        const uint8_t SUBPIXEL_RES = 100;
        uint16_t getGridWidth();
        uint16_t getGridHeight();
        uint8_t getMaxBrightness();
        uint16_t getPixelValue(uint16_t);
        uint16_t getPixelValue(uint16_t,uint16_t);
    protected:
        uint16_t gridWidth;
        uint16_t gridHeight;
    private:
        //Maximum colours supported in palette (including black at index zero)
        /* The larger the palette size, the more colours can be displayed, but palette 
         * lookup time will increase the more colours added to the palette (affecting 
         * speed of all pixel updates).
         */
        const uint16_t maxColours = 16400; //Values much over 16400 hang the Teensy3.2 I am testing on. 
        
        /* Require acceleration x and y components per panel for LED cubes. These are stored in a 12 member
         * array containg the x component for each panel [0-5] followed by the y component for each panel [6-11]
         */
        uint16_t panelAccelVectors[11];
        uint8_t maxBrightness;
        uint16_t* img; // Internal 'map' of pixels
        RGB_colour* palette;
        uint16_t coloursDefined;
        uint8_t panelSize; //Number of pixels width and height of panels (used for cube mode, which only supports square panels)
        bool cubeMode;
        
    //functions    
    public:
        RGBMatrixRenderer(uint16_t, uint16_t, uint8_t=255, bool=false);
        virtual ~RGBMatrixRenderer();
        void setPixelValue(uint16_t,uint16_t);
        void setPixelColour(uint16_t, uint16_t, RGB_colour);
        void setPixelInstant(uint16_t, uint16_t, RGB_colour);
        void updateDisplay();
        void clearImage();
        virtual void showPixels() = 0;
        virtual void msSleep(int) = 0;
        virtual void outputMessage(char[]) = 0;
        virtual int16_t random_int16(int16_t a, int16_t b) = 0;
        RGB_colour getRandomColour();
        RGB_colour newRandomColour();
        uint16_t newPositionX(uint16_t,uint16_t,bool=true);
        uint16_t newPositionY(uint16_t,uint16_t,bool=true);
        MovingPixel updatePosition(MovingPixel,bool=true);
        RGB_colour blendColour(RGB_colour,RGB_colour,uint8_t,uint8_t);
        uint16_t getColourId(RGB_colour);
        RGB_colour getColour(uint16_t);
    private:
        uint16_t newPosition(uint16_t,uint16_t,uint16_t,bool);
        uint8_t getPanel(MovingPixel);
        virtual void setPixel(uint16_t, uint16_t, RGB_colour) = 0;

}; //RGBMatrixRenderer
