/**************************************************************************************************
 * Falling Sand simulation
 * 
 * This implementation was written as a reusable animator class where the RGB matrix hardware
 * rendering class is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on the hardware.
 *
 * Based on original code by https://github.com/PaintYourDragon published as the LED sand example 
 * in the Adafruit Learning Guides here https://github.com/adafruit/Adafruit_Learning_System_Guides
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
#include <stdio.h>
#include <iostream>
#include <cmath>
#endif

#include <tuple>

#include "RGBMatrixRenderer.h"


class FallingSand
{
    //variables
    public:
    protected:
    private:
        int delayms;
        RGBMatrixRenderer &renderer;

        struct Grain {
            uint16_t  x,  y; // Position
            int16_t vx, vy; // Velocity
        };
        Grain* grains;
        uint16_t spaceMultiplier;
        uint16_t maxGrains;
        uint16_t numGrains;
        uint16_t maxX;
        uint16_t maxY;
        int16_t accelX;
        int16_t accelY;
        int16_t accelAbs;
        uint16_t shake;
        uint16_t velCap;
    //functions
    public:
        FallingSand(RGBMatrixRenderer&,uint16_t);
        ~FallingSand();
        void runCycle();
        void setAcceleration(int16_t,int16_t);
        void addGrain(RGB_colour);
        void addGrain(uint16_t,uint16_t,RGB_colour);
        void clearGrains();
        uint16_t getGrainCount();
        void imgToGrains();
    protected:
    private:
}; //FallingSand
