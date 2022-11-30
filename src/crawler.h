/**************************************************************************************************
 * Simple Crawler animator class 
 * 
 * Generates a simple animation starting at a random point in the grid, and crawling across the
 * grid at random. This is a simple example of an animator class where the RGB matrix hardware
 * rendering is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on the hardware.
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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(ARDUINO)
#include "WProgram.h"
#else
#include <iostream>
#endif

#include "RGBMatrixRenderer.h"

class Crawler
{
    //variables
    public:
        bool anyAngle;
    protected:
    private:
        RGBMatrixRenderer &renderer;
        MovingPixel leadPixel;
        uint16_t colChg = 0;
        uint16_t dirChg;
        uint16_t colChgCount = 50;
        uint16_t dirChgCount = 1;
        RGB_colour colour;
    //functions
    public:
        Crawler(RGBMatrixRenderer&,uint16_t,uint16_t,bool=false);
        ~Crawler();
        void runCycle();
    protected:
    private:

}; //Crawler