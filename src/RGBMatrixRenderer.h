/**************************************************************************************************
 * RGB Matrix Renderer abstract class 
 * 
 * This abstract class provides a template for building renderer classes to control LED Matrix
 * displays. It provides methods to set the colour of any LED/pixel in a grid, and to keep the
 * position of points within the bound of the grid (with or without wrapping over the edges).
 * A series of animation classes use the interface defined in this class to output to displays.
 * By writing a renderer class for any LED matrix hardware (or on screen display) this allows the
 * animations classes to be used with many different hardware display devices without requiring
 * code changes in the animation class.
 *
 * Copyright (C) 2020 Paul Fretwell - aka 'Footleg'
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
#endif

struct RGB_colour {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class RGBMatrixRenderer
{
    //variables
    public:
    protected:
        uint16_t gridWidth;
        uint16_t gridHeight;
    private:
        uint8_t maxBrightness;
        uint8_t* img; // Internal 'map' of pixels
        RGB_colour* palette;
        uint8_t coloursDefined;
        virtual void setPixel(uint16_t, uint16_t, RGB_colour) = 0;
        
    //functions
    public:
        RGBMatrixRenderer(uint16_t , uint16_t , uint8_t =255);
        virtual ~RGBMatrixRenderer();
        uint16_t getGridWidth();
        uint16_t getGridHeight();
        uint8_t getMaxBrightness();
        uint8_t getPixelValue(uint16_t);
        uint8_t getPixelValue(uint16_t,uint16_t);
        void setPixelValue(uint16_t,uint8_t);
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
        RGB_colour blendColour(RGB_colour,RGB_colour,uint8_t,uint8_t);
        uint8_t getColourId(RGB_colour);
        RGB_colour getColour(uint8_t);
    protected:
    private:
        uint16_t newPosition(uint16_t,uint16_t,uint16_t,bool);
}; //RGBMatrixRenderer
