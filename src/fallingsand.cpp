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
 * Added LED cube support. Acceleration is now supported in 3D coordinates, and applies differently 
 * to grains depending on which matrix panel they are on (each cube face has gravity acting in a 
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


#include "fallingsand.h"  

// default constructor
FallingSand::FallingSand(RGBMatrixRenderer &renderer_, uint16_t shake_)
    : renderer(renderer_)
{
    //Normally the grain coordinate space is 256x the pixel resolution of the pixel matrix
    //But for large displays this needs limiting to prevent the space overflowing a 16bit uint
    int maxDim = renderer.getGridWidth();
    if (renderer.getGridHeight() > maxDim) {
        maxDim = renderer.getGridHeight();
    }
    int multiplier = 5900 / maxDim;
    if (multiplier > 25) {
        spaceMultiplier = 256;
    }
    else {
        spaceMultiplier = 10 * multiplier;
    }

    char msg[44];
    sprintf(msg, "Grain coordinates space multipler = %d\n", spaceMultiplier );
    renderer.outputMessage(msg);

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
    maxX = renderer.getGridWidth()  * spaceMultiplier - 1; // Maximum X coordinate in grain space
    maxY = renderer.getGridHeight() * spaceMultiplier - 1; // Maximum Y coordinate in grain space

    velCap = spaceMultiplier;
    numGrains = 0;
    shake = shake_;
    accelX = 0;
    accelY = 0;
} //FallingSand

// default destructor
FallingSand::~FallingSand()
{
    delete [] grains;
} //~FallingSand

//Run Cycle is called once per frame of the animation
void FallingSand::runCycle()
{
    int32_t v2; // Velocity squared
    float   v;  // Absolute velocity
    int16_t shakeFactor = shake / 2;

    //Apply 2D accel vector to grain velocities...
    for(int16_t i=0; i<numGrains; i++) {
        int16_t axa = accelX + renderer.random_int16(-shakeFactor,shakeFactor+1); // A little randomness makes
        int16_t aya = accelY + renderer.random_int16(-shakeFactor,shakeFactor+1); // tall stacks topple better!
        grains[i].vx += axa;
        grains[i].vy += aya;
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
    //REMEMBER these debug messages kill the speed of the animation if more than around 10 grains!
    // char msg[80];
    // sprintf(msg, "Grain %d Vel: %d,%d; a=%d,%d\n", i, int(grains[i].vx), int(grains[i].vy), axa, aya );
    // renderer.outputMessage(msg);

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

    uint16_t i, oldidx, newidx, delta;
    uint16_t  newx, newy; //Needs to handle positions overshooting and undershooting grid space
    const int over = 10 * spaceMultiplier; //So buffer unsigned int to keep undershoots >=0
    for(i=0; i<numGrains; i++) {
        newx = grains[i].x + over + (grains[i].vx/32) ; // New position in grain space
        newy = grains[i].y + over + (grains[i].vy/32);
        if(newx > maxX + over) {         // If grain would go out of bounds
            newx = maxX + over;          // keep it inside, and
            grains[i].vx /= -2;   // give a slight bounce off the wall
        } else if(newx < over) {
            newx = over;
            grains[i].vx /= -2;
        }
        if(newy > maxY + over) {
            newy = maxY + over;
            grains[i].vy /= -2;
        } else if(newy < over) {
            newy = over;
            grains[i].vy /= -2;
        }
        //Remove overshoot buffer
        newx -= over;
        newy -= over;

        oldidx = (grains[i].y/spaceMultiplier) * renderer.getGridWidth() + (grains[i].x/spaceMultiplier); // Prior pixel #
        newidx = (newy      /spaceMultiplier) * renderer.getGridWidth() + (newx      /spaceMultiplier); // New pixel #

    //REMEMBER these debug messages kill the speed of the animation if more than around 10 grains!
    // char msg[100];
    // sprintf(msg, "Grain %d: OldIdx %d  NewIDx %d)\n", i, oldidx, newidx );
    // renderer.outputMessage(msg);

        if((oldidx != newidx) // If grain is moving to a new pixel...
            && renderer.getPixelValue(newidx) ) 
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
                    newidx = (grains[i].y / spaceMultiplier) * renderer.getGridWidth() + (newx / spaceMultiplier);
                    if(!renderer.getPixelValue(newidx)) { // That pixel's free!  Take it!  But...
                        newy         = grains[i].y; // Cancel Y motion
                        grains[i].vy /= -2;         // and bounce Y velocity
                    } else { // X pixel is taken, so try Y...
                        newidx = (newy / spaceMultiplier) * renderer.getGridWidth() + (grains[i].x / spaceMultiplier);
                        if(!renderer.getPixelValue(newidx)) { // Pixel is free, take it, but first...
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
                    newidx = (newy / spaceMultiplier) * renderer.getGridWidth() + (grains[i].x / spaceMultiplier);
                    if(!renderer.getPixelValue(newidx)) { // Pixel's free!  Take it!  But...
                        newx         = grains[i].x; // Cancel X motion
                        grains[i].vy /= -2;         // and bounce X velocity
                    } else { // Y pixel is taken, so try X...
                        newidx = (grains[i].y / spaceMultiplier) * renderer.getGridWidth() + (newx / spaceMultiplier);
                        if(!renderer.getPixelValue(newidx)) { // Pixel is free, take it, but first...
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
        
		//Update matrix memory and display state (before changing grain position as we need old position
        if (oldidx != newidx) {
            uint8_t colcode = renderer.getPixelValue(oldidx);
            renderer.setPixelValue(oldidx, 0);       // Clear old spot
            renderer.setPixelValue(newidx, colcode); // Set new spot
            renderer.setPixelInstant(grains[i].x/spaceMultiplier,grains[i].y/spaceMultiplier, renderer.getColour(0) );       //Update on screen
            renderer.setPixelInstant(newx/spaceMultiplier, newy/spaceMultiplier, renderer.getColour(colcode) ); //Update on screen
        }
        grains[i].x  = newx; // Update grain position
        grains[i].y  = newy;
//sprintf(msg, "Chang %d: %d -> %d\n", i, oldidx, newidx );
//renderer.outputMessage(msg);
    }

    //Update LEDs
    renderer.showPixels(); //Update the display (for hardware which is not instantaneous)
}

// Acceleration setter for simple 2D panel arrangements (for backwards compatibility with existing code)
void FallingSand::setAcceleration(int16_t x, int16_t y)
{
    //Acceleration direction matches axes sign. 
    //e.g. +ve accelX will cause sand grains to move to greater X positions.
    accelX = x;
    accelY = y;
    
    //Limit maximum velocity based on strength of gravity
    uint16_t maxVel = sqrt( (int32_t)x*x+(int32_t)y*y ) * spaceMultiplier / 32;
    uint16_t minVelCap = spaceMultiplier / 4;
    if (maxVel > minVelCap) {
        //Set max to 8 x magnitude of acceleration vector (set at pixel scale, so compensate for space mulipler)
        velCap = maxVel;
    }
    else {
        //Set to minimum
        velCap = minVelCap;
    }
    velCap = spaceMultiplier * 4; //Make this possible to set via a method. Needs to be * 4 for sand, but * 16 for fast particles.
    //Also need to change the scale of gravity, so fast particles crash to floor even with 1. Need fractions of this force to cause 
    //them to drift down or maybe even allow them to be 'light' particles which are less affected?

    char msg[255];
    sprintf(msg,"Acceleration set: %d,%d Vel min: %d, max: %d, cap: %d, shake=%d\n", accelX, accelY, minVelCap, maxVel, velCap, shake );
    renderer.outputMessage(msg);
}

// Acceleration setter for 3D panel arrangements (LED Cubes), sets an array of acceleration x,y components per panel
void FallingSand::setAcceleration(int16_t x, int16_t y, int16_t z)
{
    //Acceleration direction matches axes sign. 
    //e.g. +ve accelX will cause sand grains to move to greater X positions.
    //Set per panel for 6 cube faces
    accelX = x;
    accelY = y;
    
    //Limit maximum velocity based on strength of gravity
    uint16_t xyAbs = sqrt( (int32_t)x*x+(int32_t)y*y );
    uint16_t maxVel = sqrt( (int32_t)xyAbs*xyAbs+(int32_t)z*z ) * spaceMultiplier / 32;
    uint16_t minVelCap = spaceMultiplier / 4;
    if (maxVel > minVelCap) {
        //Set max to 8 x magnitude of acceleration vector (set at pixel scale, so compensate for space mulipler)
        velCap = maxVel;
    }
    else {
        //Set to minimum
        velCap = minVelCap;
    }
        

    char msg[255];
    sprintf(msg,"Acceleration set: %d,%d Vel min: %d, max: %d, cap: %d\n", accelX, accelY, minVelCap, maxVel, velCap );
    renderer.outputMessage(msg);

}

void FallingSand::addGrain(RGB_colour colour, int16_t vx, int16_t vy)
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
        // char msg[50];
        // sprintf(msg, "Random place attempt %d\n", attempts);
        // renderer.outputMessage(msg);
    } while ( (renderer.getPixelValue(y * renderer.getGridWidth() + x)) && (attempts < 2001) ); // Keep retrying until a clear spot is found
    
    //Add grain if free position was found
    if ( (renderer.getPixelValue(y * renderer.getGridWidth() + x)) == false ) {
        addGrain(x,y,colour,vx,vy);
    }
    else {
        char msg[50];
        sprintf(msg, "Failed to find free position for new grain.\n");
        renderer.outputMessage(msg);
    }

}

void FallingSand::addGrain(uint16_t x, uint16_t y, RGB_colour colour, int16_t vx, int16_t vy)
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

    grains[i].x = (x * spaceMultiplier)+renderer.random_int16(0,spaceMultiplier); // Assign position in centre of
    grains[i].y = (y * spaceMultiplier)+renderer.random_int16(0,spaceMultiplier); // the 'grain' coordinate space
    numGrains++;
    //Set initial velocity
    grains[i].vx = vx;
    grains[i].vy = vy; 
    renderer.setPixelValue( (grains[i].y / spaceMultiplier) * renderer.getGridWidth() + (grains[i].x / spaceMultiplier), renderer.getColourId(colour) ); // Mark it

char msg[100];
sprintf(msg, "Grain placed %d,%d (%d,%d) vel: %d,%d colour:%d; Total:%d\n", x,y, int(grains[i].x),int(grains[i].y), vx,vy, renderer.getColourId(colour), numGrains );
renderer.outputMessage(msg);

}

uint16_t FallingSand::getGrainCount()
{
    return numGrains;
}

void FallingSand::clearGrains()
{
    //Simply set number of grains to zero
    numGrains = 0;
}

void FallingSand::imgToGrains()
{
    //Convert all pixels in current image to grains
    for(int y=0; y<renderer.getGridHeight(); y++) {
        for(int x=0; x<renderer.getGridWidth(); x++) {
            uint8_t colcode = renderer.getPixelValue(x,y);
            if (colcode > 0) {
                addGrain(x,y,renderer.getColour(colcode));
            }
        }
    }

}