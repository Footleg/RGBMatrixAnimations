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
 */#include "fallingsand.h"

#include <iostream>
#include <cmath>

        
// default constructor
FallingSand::FallingSand(RGBMatrixRenderer &renderer_, int16_t shake_, uint16_t num_grains)
    : renderer(renderer_)
{
    // Allocate memory for pixels array
    img = new uint8_t[renderer.getGridWidth() * renderer.getGridHeight()];

    // Allocate memory for grains array
    grain = new Grain[num_grains];

    // The 'sand' grains exist in an integer coordinate space that's 256X
    // the scale of the pixel grid, allowing them to move and interact at
    // less than whole-pixel increments.
    maxX = renderer.getGridWidth()  * 256 - 1; // Maximum X coordinate in grain space
    maxY = renderer.getGridHeight() * 256 - 1; // Maximum Y coordinate in grain space

    velCap = 256;
    grainsAdded = 0;
    numGrains = num_grains;
    shake = shake_;

    //Initial random colour
    renderer.setRandomColour();

    //Clear img
    for (uint16_t i=0; i<renderer.getGridWidth() * renderer.getGridHeight(); i++) {
        img[i]=0;
    }

} //FallingSand

// default destructor
FallingSand::~FallingSand()
{
    delete [] img;
} //~FallingSand

//Run Cycle is called once per frame of the animation
void FallingSand::runCycle()
{
    // Update pixel data on display
    for(int y=0; y<renderer.getGridHeight(); y++) {
//        char msg[255];
//        std::string line = "";
        for(int x=0; x<renderer.getGridWidth(); x++) {
//            line += std::to_string((img[y*renderer.getGridWidth() + x])) + ",";
            uint8_t colcode = img[y*renderer.getGridWidth() + x];
            if (colcode) {
                /**/
                uint8_t b = (colcode%6)*51;
                uint8_t g = (int(colcode/6)%6)*51;
                uint8_t r = (int(colcode/36)%6)*51;

                renderer.setPixel(x,y,r,g,b);
                
                //renderer.setPixel(x,y,renderer.r,renderer.g,renderer.b);
//                sprintf(msg,"Pixel at %d\n",int(y*renderer.getGridWidth() + x) );
//                renderer.outputMessage(msg);
            }
            else {
                renderer.setPixel(x,y,0,0,0);
            }
        }
//        sprintf(msg,"%s\n", const_cast<char*>(line.c_str()) );
//        renderer.outputMessage(msg);
    }
    
    // Read accelerometer...
    int16_t ax = -accelX,      // Transform accelerometer axes
            ay =  accelY,      // to grain coordinate space
            az = shake;        // Random motion factor

    //az = (az >= 300) ? 1 : 400 - az;      // Clip & invert
    ax -= az;                         // Subtract motion factor from X, Y
    ay -= az;
    int16_t az2 = az * 2 + 1;         // Range of random motion to add back in

    // ...and apply 2D accel vector to grain velocities...
    int32_t v2; // Velocity squared
    float   v;  // Absolute velocity
    for(int16_t i=0; i<numGrains; i++) {
        grain[i].vx += ax + renderer.random_uint(0,az2); // A little randomness makes
        grain[i].vy += ay + renderer.random_uint(0,az2); // tall stacks topple better!
        // Terminal velocity (in any direction) is 256 units -- equal to
        // 1 pixel -- which keeps moving grains from passing through each other
        // and other such mayhem.  Though it takes some extra math, velocity is
        // clipped as a 2D vector (not separately-limited X & Y) so that
        // diagonal movement isn't faster
        v2 = (int32_t)grain[i].vx*grain[i].vx+(int32_t)grain[i].vy*grain[i].vy;

        if(v2 > (velCap*velCap) ) { // If v^2 > 65536, then v > 256
            v = sqrt((float)v2); // Velocity vector magnitude
            grain[i].vx = (int16_t)(velCap*(float)grain[i].vx/v); // Maintain heading
            grain[i].vy = (int16_t)(velCap*(float)grain[i].vy/v); // Limit magnitude
        }
if (i<0) {
    char msg[80];
    sprintf(msg, "Grain %d Vel: %d,%d; a=%d,%d,%d az2=%d\n", i, int(grain[i].vx), int(grain[i].vy), ax, ay, az, az2 );
    renderer.outputMessage(msg);
}
    }
        
    // ...then update position of each grain, one at a time, checking for
    // collisions and having them react.  This really seems like it shouldn't
    // work, as only one grain is considered at a time while the rest are
    // regarded as stationary.  Yet this naive algorithm, taking many not-
    // technically-quite-correct steps, and repeated quickly enough,
    // visually integrates into something that somewhat resembles physics.
    // (I'd initially tried implementing this as a bunch of concurrent and
    // "realistic" elastic collisions among circular grains, but the
    // calculations and volument of code quickly got out of hand for both
    // the tiny 8-bit AVR microcontroller and my tiny dinosaur brain.)

    uint16_t        i, oldidx, newidx, delta;
    int16_t        newx, newy;
    
    for(i=0; i<numGrains; i++) {
        newx = grain[i].x + (grain[i].vx/32); // New position in grain space
        newy = grain[i].y + (grain[i].vy/32);
        if(newx > maxX) {               // If grain would go out of bounds
            newx         = maxX;          // keep it inside, and
            grain[i].vx /= -2;             // give a slight bounce off the wall
        } else if(newx < 0) {
            newx         = 0;
            grain[i].vx /= -2;
        }
        if(newy > maxY) {
            newy         = maxY;
            grain[i].vy /= -2;
        } else if(newy < 0) {
            newy         = 0;
            grain[i].vy /= -2;
        }

        oldidx = (grain[i].y/256) * renderer.getGridWidth() + (grain[i].x/256); // Prior pixel #
        newidx = (newy      /256) * renderer.getGridWidth() + (newx      /256); // New pixel #
//char msg[100];
//sprintf(msg, "Grain %d: Old %d  (Calc %d)\n", i, oldidx, (grain[i].y/256) * renderer.getGridWidth() + (grain[i].x/256) );
//renderer.outputMessage(msg);
        if((oldidx != newidx) && // If grain is moving to a new pixel...
            img[newidx]) 
        {       // but if that pixel is already occupied...
            delta = abs(newidx - oldidx); // What direction when blocked?
            if(delta == 1) {            // 1 pixel left or right)
                newx         = grain[i].x;  // Cancel X motion
                grain[i].vx /= -2;          // and bounce X velocity (Y is OK)
                newidx       = oldidx;      // No pixel change
            } else if(delta == renderer.getGridWidth()) { // 1 pixel up or down
                newy         = grain[i].y;  // Cancel Y motion
                grain[i].vy /= -2;          // and bounce Y velocity (X is OK)
                newidx       = oldidx;      // No pixel change
            } else { // Diagonal intersection is more tricky...
                // Try skidding along just one axis of motion if possible (start w/
                // faster axis).  Because we've already established that diagonal
                // (both-axis) motion is occurring, moving on either axis alone WILL
                // change the pixel index, no need to check that again.
                if((abs(grain[i].vx) - abs(grain[i].vy)) >= 0) { // X axis is faster
                    newidx = (grain[i].y / 256) * renderer.getGridWidth() + (newx / 256);
                    if(!img[newidx]) { // That pixel's free!  Take it!  But...
                        newy         = grain[i].y; // Cancel Y motion
                        grain[i].vy /= -2;         // and bounce Y velocity
                    } else { // X pixel is taken, so try Y...
                        newidx = (newy / 256) * renderer.getGridWidth() + (grain[i].x / 256);
                        if(!img[newidx]) { // Pixel is free, take it, but first...
                        newx         = grain[i].x; // Cancel X motion
                        grain[i].vx /= -2;         // and bounce X velocity
                        } else { // Both spots are occupied
                        newx         = grain[i].x; // Cancel X & Y motion
                        newy         = grain[i].y;
                        grain[i].vx /= -2;         // Bounce X & Y velocity
                        grain[i].vy /= -2;
                        newidx       = oldidx;     // Not moving
                        }
                    }
                } else { // Y axis is faster, start there
                    newidx = (newy / 256) * renderer.getGridWidth() + (grain[i].x / 256);
                    if(!img[newidx]) { // Pixel's free!  Take it!  But...
                        newx         = grain[i].x; // Cancel X motion
                        grain[i].vy /= -2;         // and bounce X velocity
                    } else { // Y pixel is taken, so try X...
                        newidx = (grain[i].y / 256) * renderer.getGridWidth() + (newx / 256);
                        if(!img[newidx]) { // Pixel is free, take it, but first...
                            newy         = grain[i].y; // Cancel Y motion
                            grain[i].vy /= -2;         // and bounce Y velocity
                        } else { // Both spots are occupied
                            newx         = grain[i].x; // Cancel X & Y motion
                            newy         = grain[i].y;
                            grain[i].vx /= -2;         // Bounce X & Y velocity
                            grain[i].vy /= -2;
                            newidx       = oldidx;     // Not moving
                        }
                    }
                }
            }
        }
        grain[i].x  = newx; // Update grain position
        grain[i].y  = newy;
        if (oldidx != newidx) {
            uint8_t colcode = img[oldidx];
            img[oldidx] = 0;       // Clear old spot
            img[newidx] = colcode; // Set new spot
        }
//sprintf(msg, "Chang %d: %d -> %d\n", i, oldidx, newidx );
//renderer.outputMessage(msg);
    }
}

void FallingSand::setAcceleration(int16_t x, int16_t y)
{
    accelX = x;
    accelY = y;
    
    //Limit maximum velocity based on strength of gravity
    uint16_t maxVel = sqrt( (int32_t)x*x+(int32_t)y*y ) * 5;
    const int16_t minVelCap = 256;
    if (maxVel > minVelCap)
        velCap = maxVel;
    else
        velCap = minVelCap;

    char msg[255];
    sprintf(msg,"Acceleration set: %d,%d Max: %f\n", accelX, accelY, velCap );
    renderer.outputMessage(msg);
}

void FallingSand::addGrain(uint8_t id)
{
    //Place grains into array.
    //id indicates grain colour (currently based on 6 values each per r,g,b channel
    //so values from 1-215 represent colours)
    uint16_t i = grainsAdded;
    do {
        grain[i].x = renderer.random_uint(0,renderer.getGridWidth()  * 256); // Assign random position within
        grain[i].y = renderer.random_uint(0,renderer.getGridHeight() * 256); // the 'grain' coordinate space
        // Check if corresponding pixel position is already occupied...
    } while(img[(grain[i].y / 256) * renderer.getGridWidth() + (grain[i].x / 256)]); // Keep retrying until a clear spot is found
    grainsAdded++;
    grain[i].vx = grain[i].vy = 0; // Initial velocity is zero
    img[(grain[i].y / 256) * renderer.getGridWidth() + (grain[i].x / 256)] = id; // Mark it

char msg[100];
sprintf(msg, "Grains placed %d,%d colour:%d\n", int(grain[i].x), int(grain[i].y), int(img[(grain[i].y / 256) * renderer.getGridWidth() + (grain[i].x / 256)]) );
renderer.outputMessage(msg);

}

void FallingSand::setStaticPixel(uint16_t x, uint16_t y, uint8_t id)
{
    img[y * renderer.getGridWidth() + x] = id; // Mark it
}