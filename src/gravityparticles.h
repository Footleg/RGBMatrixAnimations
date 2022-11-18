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
 * The original 'sand particles' idea has been extended to simulate other particle behaviours
 * including 'sparks' which are much lighter and can fracture into multiple new particles as they
 * decay in speed and brightness. 
 * 
 * Added LED cube support. Acceleration is now supported in 3D coordinates, and applies differently 
 * to particles depending on which matrix panel they are on (each cube face has gravity acting in a 
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
#endif

#include <tuple>

#include "RGBMatrixRenderer.h"


class GravityParticles
{
    //variables
    public:
        struct Particle {
            uint16_t  x,  y; // Position
            int16_t vx, vy; // Velocity
        };
    protected:
    private:
        int delayms;
        RGBMatrixRenderer &renderer;
        Particle* particles;
        uint16_t spaceMultiplier;
        uint16_t maxParticles;
        uint16_t numParticles;
        uint16_t maxX;
        uint16_t maxY;
        int16_t accelX;
        int16_t accelY;
        int16_t accelAbs;
        uint16_t shake;
        uint16_t velCap;
        float_t loss;
        uint8_t bounce;
    //functions
    public:
        GravityParticles(RGBMatrixRenderer&,uint16_t,uint8_t=10);
        ~GravityParticles();
        void runCycle();
        void setAcceleration(int16_t,int16_t);
        void setAcceleration(int16_t,int16_t,int16_t);
        void addParticle(RGB_colour,int16_t=0,int16_t=0);
        void addParticle(uint16_t,uint16_t,RGB_colour,int16_t=0,int16_t=0);
        Particle deleteParticle(uint16_t);
        Particle getParticle(uint16_t);
        void clearParticles();
        uint16_t getParticleCount();
        void imgToParticles();
    protected:
    private:
}; //GravityParticles
