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

#include <iostream>
#include <cmath>

#include "fallingsand.h"  

// default constructor
FallingSand::FallingSand(RGBMatrixRenderer &renderer_, int16_t shake_)
    : renderer(renderer_)
{
    // Allocate memory for colour palette array
    palette = new RGB_colour[255];
    coloursDefined = 0;
    
    // Allocate memory for pixels array
    img = new uint8_t[renderer.getGridWidth() * renderer.getGridHeight()];

    // Allocate initial memory for grains array
    uint16_t max = renderer.getGridWidth() * renderer.getGridHeight();
    if (max < 100) {
        maxGrains =  max;
    }
    else {
        maxGrains = 100;
    }
    
    grains = new Grain[maxGrains];

    // The 'sand' grains exist in an integer coordinate space that's 256X
    // the scale of the pixel grid, allowing them to move and interact at
    // less than whole-pixel increments.
    maxX = renderer.getGridWidth()  * 256 - 1; // Maximum X coordinate in grain space
    maxY = renderer.getGridHeight() * 256 - 1; // Maximum Y coordinate in grain space

    velCap = 256;
    numGrains = 0;
    shake = shake_;

    //Clear img
    for (uint16_t i=0; i<renderer.getGridWidth() * renderer.getGridHeight(); i++) {
        img[i]=0;
    }

} //FallingSand

// default destructor
FallingSand::~FallingSand()
{
    delete [] palette;
    delete [] img;
    delete [] grains;
} //~FallingSand

//Update Display
void FallingSand::updateDisplay()
{
    // Update pixel data on display
    for(int y=0; y<renderer.getGridHeight(); y++) {
        for(int x=0; x<renderer.getGridWidth(); x++) {
            uint8_t colcode = img[y*renderer.getGridWidth() + x];
            if (colcode) {
                renderer.setPixel(x,y,getColour(colcode));
            }
            else {
                renderer.setPixel(x,y,RGB_colour{0,0,0});
            }
        }
    }
}

//Run Cycle is called once per frame of the animation
void FallingSand::runCycle()
{
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
        grains[i].vx += ax + renderer.random_int16(0,az2); // A little randomness makes
        grains[i].vy += ay + renderer.random_int16(0,az2); // tall stacks topple better!
        // Terminal velocity (in any direction) is 256 units -- equal to
        // 1 pixel -- which keeps moving grains from passing through each other
        // and other such mayhem.  Though it takes some extra math, velocity is
        // clipped as a 2D vector (not separately-limited X & Y) so that
        // diagonal movement isn't faster
        v2 = (int32_t)grains[i].vx*grains[i].vx+(int32_t)grains[i].vy*grains[i].vy;

        if(v2 > (velCap*velCap) ) { // If v^2 > 65536, then v > 256
            v = sqrt((float)v2); // Velocity vector magnitude
            grains[i].vx = (int16_t)(velCap*(float)grains[i].vx/v); // Maintain heading
            grains[i].vy = (int16_t)(velCap*(float)grains[i].vy/v); // Limit magnitude
        }
if (i<0) {
    char msg[80];
    sprintf(msg, "Grain %d Vel: %d,%d; a=%d,%d,%d az2=%d\n", i, int(grains[i].vx), int(grains[i].vy), ax, ay, az, az2 );
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
        newx = grains[i].x + (grains[i].vx/32); // New position in grain space
        newy = grains[i].y + (grains[i].vy/32);
        if(newx > maxX) {               // If grain would go out of bounds
            newx         = maxX;          // keep it inside, and
            grains[i].vx /= -2;             // give a slight bounce off the wall
        } else if(newx < 0) {
            newx         = 0;
            grains[i].vx /= -2;
        }
        if(newy > maxY) {
            newy         = maxY;
            grains[i].vy /= -2;
        } else if(newy < 0) {
            newy         = 0;
            grains[i].vy /= -2;
        }

        oldidx = (grains[i].y/256) * renderer.getGridWidth() + (grains[i].x/256); // Prior pixel #
        newidx = (newy      /256) * renderer.getGridWidth() + (newx      /256); // New pixel #
//char msg[100];
//sprintf(msg, "Grain %d: Old %d  (Calc %d)\n", i, oldidx, (grains[i].y/256) * renderer.getGridWidth() + (grains[i].x/256) );
//renderer.outputMessage(msg);
        if((oldidx != newidx) && // If grain is moving to a new pixel...
            img[newidx]) 
        {       // but if that pixel is already occupied...
            delta = abs(newidx - oldidx); // What direction when blocked?
            if(delta == 1) {            // 1 pixel left or right)
                newx         = grains[i].x;  // Cancel X motion
                grains[i].vx /= -2;          // and bounce X velocity (Y is OK)
                newidx       = oldidx;      // No pixel change
            } else if(delta == renderer.getGridWidth()) { // 1 pixel up or down
                newy         = grains[i].y;  // Cancel Y motion
                grains[i].vy /= -2;          // and bounce Y velocity (X is OK)
                newidx       = oldidx;      // No pixel change
            } else { // Diagonal intersection is more tricky...
                // Try skidding along just one axis of motion if possible (start w/
                // faster axis).  Because we've already established that diagonal
                // (both-axis) motion is occurring, moving on either axis alone WILL
                // change the pixel index, no need to check that again.
                if((abs(grains[i].vx) - abs(grains[i].vy)) >= 0) { // X axis is faster
                    newidx = (grains[i].y / 256) * renderer.getGridWidth() + (newx / 256);
                    if(!img[newidx]) { // That pixel's free!  Take it!  But...
                        newy         = grains[i].y; // Cancel Y motion
                        grains[i].vy /= -2;         // and bounce Y velocity
                    } else { // X pixel is taken, so try Y...
                        newidx = (newy / 256) * renderer.getGridWidth() + (grains[i].x / 256);
                        if(!img[newidx]) { // Pixel is free, take it, but first...
                        newx         = grains[i].x; // Cancel X motion
                        grains[i].vx /= -2;         // and bounce X velocity
                        } else { // Both spots are occupied
                        newx         = grains[i].x; // Cancel X & Y motion
                        newy         = grains[i].y;
                        grains[i].vx /= -2;         // Bounce X & Y velocity
                        grains[i].vy /= -2;
                        newidx       = oldidx;     // Not moving
                        }
                    }
                } else { // Y axis is faster, start there
                    newidx = (newy / 256) * renderer.getGridWidth() + (grains[i].x / 256);
                    if(!img[newidx]) { // Pixel's free!  Take it!  But...
                        newx         = grains[i].x; // Cancel X motion
                        grains[i].vy /= -2;         // and bounce X velocity
                    } else { // Y pixel is taken, so try X...
                        newidx = (grains[i].y / 256) * renderer.getGridWidth() + (newx / 256);
                        if(!img[newidx]) { // Pixel is free, take it, but first...
                            newy         = grains[i].y; // Cancel Y motion
                            grains[i].vy /= -2;         // and bounce Y velocity
                        } else { // Both spots are occupied
                            newx         = grains[i].x; // Cancel X & Y motion
                            newy         = grains[i].y;
                            grains[i].vx /= -2;         // Bounce X & Y velocity
                            grains[i].vy /= -2;
                            newidx       = oldidx;     // Not moving
                        }
                    }
                }
            }
        }
        grains[i].x  = newx; // Update grain position
        grains[i].y  = newy;
        if (oldidx != newidx) {
            uint8_t colcode = img[oldidx];
            img[oldidx] = 0;       // Clear old spot
            img[newidx] = colcode; // Set new spot
        }
//sprintf(msg, "Chang %d: %d -> %d\n", i, oldidx, newidx );
//renderer.outputMessage(msg);
    }

    //Update LEDs
    updateDisplay();
}

void FallingSand::setAcceleration(int16_t x, int16_t y)
{
    accelX = x;
    accelY = y;
    
    //Limit maximum velocity based on strength of gravity
    uint16_t maxVel = sqrt( (int32_t)x*x+(int32_t)y*y ) * 8;
    const int16_t minVelCap = 64;
    if (maxVel > minVelCap)
        velCap = maxVel;
    else
        velCap = minVelCap;

    char msg[255];
    sprintf(msg,"Acceleration set: %d,%d Max: %f\n", accelX, accelY, velCap );
    renderer.outputMessage(msg);
}

void FallingSand::addGrain(RGB_colour colour)
{
    //Place grain in random free position.
    //id indicates grain colour (currently based on 6 values each per r,g,b channel
    //so values from 1-215 represent colours)
    uint16_t x,y;
    uint16_t attempts = 0;
    do {
        x = renderer.random_int16(0,renderer.getGridWidth() ); // Assign random position within
        y = renderer.random_int16(0,renderer.getGridHeight() ); // the 'grain' coordinate space
        attempts++;
        // Check if corresponding pixel position is already occupied...
/*        char msg[50];
        sprintf(msg, "Random place attempt %d\n", attempts);
        renderer.outputMessage(msg);*/
    } while ( (img[y * renderer.getGridWidth() + x]) && (attempts < 2001) ); // Keep retrying until a clear spot is found
    
    //Add grain if free position was found
    if ( (img[y * renderer.getGridWidth() + x]) == false ) {
        addGrain(x,y,colour);
    }
    else {
        char msg[50];
        sprintf(msg, "Failed to find free position for new grain.\n");
        renderer.outputMessage(msg);
    }

}

void FallingSand::addGrain(uint16_t x, uint16_t y, RGB_colour colour)
{
    //Place grain into array at specified position.
    uint16_t i = numGrains;

    //Check for grains array overflow
    if (i == maxGrains) {
        //Expand grains array
        Grain* newArr = new Grain[maxGrains + 20];
        for (uint16_t i = 0; i < maxGrains; i++) {
            newArr[i] = grains[i];
        }
        delete[] grains;
        grains = newArr;
        maxGrains += 20;
        char msg[50];
        sprintf(msg, "Grain store expanded to size %d\n", maxGrains);
        renderer.outputMessage(msg);
    }

    grains[i].x = (x * 256)+renderer.random_int16(0,255); // Assign position in centre of
    grains[i].y = (y * 256)+renderer.random_int16(0,255); // the 'grain' coordinate space
    numGrains++;
    grains[i].vx = grains[i].vy = 0; // Initial velocity is zero
    img[(grains[i].y / 256) * renderer.getGridWidth() + (grains[i].x / 256)] = getColourId(colour); // Mark it
/*
char msg[100];
sprintf(msg, "Grains placed %d,%d colour:%d; Total:%d\n", int(grains[i].x), int(grains[i].y), int(img[(grains[i].y / 256) * renderer.getGridWidth() + (grains[i].x / 256)]), numGrains );
renderer.outputMessage(msg);
*/
}

uint16_t FallingSand::getGrainCount()
{
    return numGrains;
}

void FallingSand::setStaticPixel(uint16_t x, uint16_t y, RGB_colour colour)
{
    img[y * renderer.getGridWidth() + x] = getColourId(colour); // Mark it
}

void FallingSand::clearGrains()
{
    //Simply set number of grains to zero
    numGrains = 0;
}

uint8_t FallingSand::getColourId(RGB_colour colour)
{
    //Search palette for matching colour
    uint8_t id = 0;
    for (uint8_t i=1; i<coloursDefined+1; i++) {
        if ( (palette[i].r == colour.r)
          && (palette[i].g == colour.g) 
          && (palette[i].b == colour.b) ) {
            id = i;
            break;
        }
    }

    //If match not found, add to palette if room
    if (id == 0) {
        if (coloursDefined < 255) {
            coloursDefined++;
            palette[coloursDefined] = colour;
        }
        id = coloursDefined; //For now set to last colour even if we couldn't add another one
    }

    return id;
}

RGB_colour FallingSand::getColour(uint8_t id)
{
    RGB_colour colour = {0,0,0};

    if (id <= coloursDefined) {
        colour = palette[id];
    }

    return colour;
}

void FallingSand::imgToGrains()
{
    //Convert all pixels in current image to grains
    for(int y=0; y<renderer.getGridHeight(); y++) {
        for(int x=0; x<renderer.getGridWidth(); x++) {
            uint8_t colcode = img[y*renderer.getGridWidth() + x];
            if (colcode > 0) {
                addGrain(x,y,getColour(colcode));
            }
        }
    }

}