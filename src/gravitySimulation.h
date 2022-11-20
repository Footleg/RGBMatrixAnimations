/**************************************************************************************************
 * Particles simulation
 * 
 * This implementation was written as a reusable animator class where the RGB matrix hardware
 * rendering class is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on your specific hardware.
 *
 * Based on original code by https://github.com/PaintYourDragon published as the LED sand example 
 * in the Adafruit Learning Guides here https://github.com/adafruit/Adafruit_Learning_System_Guides
 * 
 * The original 'sand balls' idea has been extended to simulate other particle behaviours
 * including 'sparks' which are much lighter and can fracture into multiple new balls as they
 * decay in speed and brightness. 
 * 
 * Added LED cube support. Acceleration is now supported in 3D coordinates, and applies differently 
 * to balls depending on which matrix panel they are on (each cube face has gravity acting in a 
 * different direction with respect to the X and Y directions on that panel).
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
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>
#endif

#include <tuple>

#include "RGBMatrixRenderer.h"

using std::vector;

class GravitySimulation
{
    //variables
    public:
        struct Ball {
            float     x;
            float     y;
            uint8_t   r;
            float     dx;
            float     dy;
            RGB_colour colour;
        };
        float forcePower = 2.0f;
    protected:
    private:
        int delayms;
        RGBMatrixRenderer &renderer;
        uint16_t spaceMultiplier;
        uint16_t maxParticles;
        uint16_t numBalls;
        uint16_t maxX;
        uint16_t maxY;
        vector<Ball> shapes;
        uint8_t mode = 0;
        float minX = 0;
        float minY = 0;
        uint8_t maxRadius;
    //functions
    public:
        GravitySimulation(RGBMatrixRenderer&,uint8_t);
        ~GravitySimulation();
        void runCycle();
        void addBall();
        void setMode(uint8_t);
    protected:
    private:
        Ball createBall();
}; //GravitySimulation
