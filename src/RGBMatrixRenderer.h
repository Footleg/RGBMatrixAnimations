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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(ARDUINO)
#include "WProgram.h"
#else
#include <stdint.h>
#include <stdio.h>
#endif


class RGBMatrixRenderer
{
    //variables
    public:
        int r;
        int g;
        int b;
    protected:
        int gridWidth;
        int gridHeight;
    private:
        int maxBrightness;
        
    //functions
    public:
        RGBMatrixRenderer(int, int, int=255);
        virtual ~RGBMatrixRenderer();
        int getGridWidth();
        int getGridHeight();
        uint8_t getMaxBrightness();
        virtual void setPixel(int, int, uint8_t, uint8_t, uint8_t) = 0;
        virtual void showPixels() = 0;
        virtual void msSleep(int) = 0;
        virtual void outputMessage(char[]) = 0;
        virtual uint16_t random_uint(uint16_t,uint16_t) = 0;
        void setRandomColour();
        int newPositionX(int,int,bool=true);
        int newPositionY(int,int,bool=true);
        uint8_t blendColour(uint8_t,uint8_t,uint8_t,uint8_t);
    protected:
    private:
        int newPosition(int,int,int,bool);
}; //RGBMatrixRenderer
