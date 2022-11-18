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


#include "gravityparticles.h"  

// default constructor
GravityParticles::GravityParticles(RGBMatrixRenderer &renderer_, uint16_t shake_, uint8_t bounce_)
    : renderer(renderer_)
{
    //Normally the particle coordinate space is 256x the pixel resolution of the pixel matrix
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

    // Allocate initial memory for particles array
    uint16_t max = renderer.getGridWidth() * renderer.getGridHeight();
    if (max < 100) {
        maxParticles =  max;
    }
    else {
        maxParticles = 100;
    }
    
    particles = new Particle[maxParticles];

    // The 'sand' particles exist in an integer coordinate space that's 256X
    // the scale of the pixel grid, allowing them to move and interact at
    // less than whole-pixel increments.
    maxX = renderer.getGridWidth()  * spaceMultiplier - 1; // Maximum X coordinate in particle space
    maxY = renderer.getGridHeight() * spaceMultiplier - 1; // Maximum Y coordinate in particle space

    velCap = spaceMultiplier * 64; //Make this possible to set via a method. Needs to be * 4 for sand, but * 16 for fast particles.

    numParticles = 0;
    shake = shake_;
    bounce = bounce_;
    accelX = 0;
    accelY = 0;

    //Loss should be between 1 - 6. If should not be < 1 and particles will gain energy from collisions then
    loss = 1.0+float_t(255-bounce_)*5/255;    

    char msg[64];
    sprintf(msg, "Particle coordinates space multipler = %d, Loss=%.2f, b=%d\n", spaceMultiplier, loss, bounce );
    renderer.outputMessage(msg);

} //GravityParticles

// default destructor
GravityParticles::~GravityParticles()
{
    delete [] particles;
} //~GravityParticles

//Run Cycle is called once per frame of the animation
void GravityParticles::runCycle()
{
    int32_t v2; // Velocity squared
    float   v;  // Absolute velocity
    int16_t shakeFactor = shake / 2;

    //Apply 2D accel vector to particle velocities...
    for(uint16_t i=0; i<numParticles; i++) {
        int16_t axa = accelX + renderer.random_int16(-shakeFactor,shakeFactor+1); // A little randomness makes
        int16_t aya = accelY + renderer.random_int16(-shakeFactor,shakeFactor+1); // tall stacks topple better!
        particles[i].vx += axa;
        particles[i].vy += aya;
        // Terminal velocity (in any direction) is 256 units -- equal to
        // 1 pixel -- which keeps moving particles from passing through each other
        // and other such mayhem.  Though it takes some extra math, velocity is
        // clipped as a 2D vector (not separately-limited X & Y) so that
        // diagonal movement isn't faster
        v2 = (int32_t)particles[i].vx*particles[i].vx+(int32_t)particles[i].vy*particles[i].vy;

        if(v2 > (velCap*velCap) ) { // If v^2 > 65536, then v > 256
            v = sqrt((float)v2); // Velocity vector magnitude
            particles[i].vx = (int16_t)(velCap*(float)particles[i].vx/v); // Maintain heading
            particles[i].vy = (int16_t)(velCap*(float)particles[i].vy/v); // Limit magnitude
        }
    //REMEMBER these debug messages kill the speed of the animation if more than around 10 particles!
    // char msg[80];
    // sprintf(msg, "Particle %d Vel: %d,%d; a=%d,%d\n", i, int(particles[i].vx), int(particles[i].vy), axa, aya );
    // renderer.outputMessage(msg);

    }
        
    // ...then update position of each particle, one at a time, checking for
    // collisions and having them react.  This really seems like it shouldn't
    // work, as only one particle is considered at a time while the rest are
    // regarded as stationary.  Yet this naive algorithm, taking many not-
    // technically-quite-correct steps, and repeated quickly enough,
    // visually integrates into something that somewhat resembles physics.
    // (I'd initially tried implementing this as a bunch of concurrent and
    // "realistic" elastic collisions among circular particles, but the
    // calculations and volument of code quickly got out of hand for both
    // the tiny 8-bit AVR microcontroller and my tiny dinosaur brain.)

    uint16_t i, oldidx, newidx, delta;
    uint16_t  newx, newy; //Needs to handle positions overshooting and undershooting grid space
    const int over = 10 * spaceMultiplier; //Add buffer amount to unsigned integers, to keep undershoots >=0
    //const float loss = 1.2; //How much velocity is divided by on each collision
    const int velDiv = 256; //Amount that velocity is divided by when applied to position
    for(i=0; i<numParticles; i++) {
        newx = particles[i].x + over + (particles[i].vx/velDiv) ; // New position in particle space
        newy = particles[i].y + over + (particles[i].vy/velDiv);
        if(newx > maxX + over) {         // If particle would go out of bounds
            newx = maxX + over;          // keep it inside, and
            if ( bounce > 0 ) {
                particles[i].vx /= -loss;   // give a slight bounce off the wall
            }
            else {
                particles[i].vx = 0;        // Stop it dead if no bounce
            }
        } else if(newx < over) {
            newx = over;
            if ( bounce > 0 ) {
                particles[i].vx /= -loss;   // give a slight bounce off the wall
            }
            else {
                particles[i].vx = 0;        // Stop it dead if no bounce
            }
        }
        if(newy > maxY + over) {
            newy = maxY + over;
            if ( bounce > 0 ) {
                particles[i].vy /= -loss;   // give a slight bounce off the wall
            }
            else {
                particles[i].vy = 0;        // Stop it dead if no bounce
            }
        } else if(newy < over) {
            newy = over;
            if ( bounce > 0 ) {
                particles[i].vy /= -loss;   // give a slight bounce off the wall
                // char msg[100];
                // sprintf(msg, "Particle %d: vy %d  bounce %d)\n", i, particles[i].vy, bounce );
                // renderer.outputMessage(msg);
            }
            else {
                particles[i].vy = 0;        // Stop it dead if no bounce
                // char msg[100];
                // sprintf(msg, "Particle %d: vy %d  bounce %d)\n", i, particles[i].vy, bounce );
                // renderer.outputMessage(msg);
            }
        }
        //Remove overshoot buffer
        newx -= over;
        newy -= over;

        oldidx = (particles[i].y/spaceMultiplier) * renderer.getGridWidth() + (particles[i].x/spaceMultiplier); // Prior pixel #
        newidx = (newy      /spaceMultiplier) * renderer.getGridWidth() + (newx      /spaceMultiplier); // New pixel #

    //REMEMBER these debug messages kill the speed of the animation if more than around 10 particles!
    // char msg[100];
    // sprintf(msg, "Particle %d: OldIdx %d  NewIDx %d)\n", i, oldidx, newidx );
    // renderer.outputMessage(msg);

        if((oldidx != newidx) // If particle is moving to a new pixel...
            && renderer.getPixelValue(newidx) ) 
        {       // but if that pixel is already occupied...
            delta = abs(newidx - oldidx); // What direction when blocked?
            if(delta == 1) {            // 1 pixel left or right)
                newx         = particles[i].x;  // Cancel X motion
                particles[i].vx /= -loss;          // and bounce X velocity (Y is OK)
                newidx       = oldidx;      // No pixel change
            } else if(delta == renderer.getGridWidth()) { // 1 pixel up or down
                newy         = particles[i].y;  // Cancel Y motion
                particles[i].vy /= -loss;          // and bounce Y velocity (X is OK)
                newidx       = oldidx;      // No pixel change
            } else { // Diagonal intersection is more tricky...
                // Try skidding along just one axis of motion if possible (start w/
                // faster axis).  Because we've already established that diagonal
                // (both-axis) motion is occurring, moving on either axis alone WILL
                // change the pixel index, no need to check that again.
                if((abs(particles[i].vx) - abs(particles[i].vy)) >= 0) { // X axis is faster
                    newidx = (particles[i].y / spaceMultiplier) * renderer.getGridWidth() + (newx / spaceMultiplier);
                    if(!renderer.getPixelValue(newidx)) { // That pixel's free!  Take it!  But...
                        newy         = particles[i].y; // Cancel Y motion
                        particles[i].vy /= -loss;         // and bounce Y velocity
                    } else { // X pixel is taken, so try Y...
                        newidx = (newy / spaceMultiplier) * renderer.getGridWidth() + (particles[i].x / spaceMultiplier);
                        if(!renderer.getPixelValue(newidx)) { // Pixel is free, take it, but first...
                        newx         = particles[i].x; // Cancel X motion
                        particles[i].vx /= -loss;         // and bounce X velocity
                        } else { // Both spots are occupied
                        newx         = particles[i].x; // Cancel X & Y motion
                        newy         = particles[i].y;
                        particles[i].vx /= -loss;         // Bounce X & Y velocity
                        particles[i].vy /= -loss;
                        newidx       = oldidx;     // Not moving
                        }
                    }
                } else { // Y axis is faster, start there
                    newidx = (newy / spaceMultiplier) * renderer.getGridWidth() + (particles[i].x / spaceMultiplier);
                    if(!renderer.getPixelValue(newidx)) { // Pixel's free!  Take it!  But...
                        newx         = particles[i].x; // Cancel X motion
                        particles[i].vy /= -loss;         // and bounce X velocity
                    } else { // Y pixel is taken, so try X...
                        newidx = (particles[i].y / spaceMultiplier) * renderer.getGridWidth() + (newx / spaceMultiplier);
                        if(!renderer.getPixelValue(newidx)) { // Pixel is free, take it, but first...
                            newy         = particles[i].y; // Cancel Y motion
                            particles[i].vy /= -loss;         // and bounce Y velocity
                        } else { // Both spots are occupied
                            newx         = particles[i].x; // Cancel X & Y motion
                            newy         = particles[i].y;
                            particles[i].vx /= -loss;         // Bounce X & Y velocity
                            particles[i].vy /= -loss;
                            newidx       = oldidx;     // Not moving
                        }
                    }
                }
            }
        }
        
		//Update matrix memory and display state (before changing particle position as we need old position
        if (oldidx != newidx) {
            uint16_t colcode = renderer.getPixelValue(oldidx);
            renderer.setPixelValue(oldidx, 0);       // Clear old spot
            renderer.setPixelValue(newidx, colcode); // Set new spot
            renderer.setPixelInstant(particles[i].x/spaceMultiplier,particles[i].y/spaceMultiplier, renderer.getColour(0) );       //Update on screen
            renderer.setPixelInstant(newx/spaceMultiplier, newy/spaceMultiplier, renderer.getColour(colcode) ); //Update on screen
        }
        particles[i].x  = newx; // Update particle position
        particles[i].y  = newy;
//sprintf(msg, "Chang %d: %d -> %d\n", i, oldidx, newidx );
//renderer.outputMessage(msg);
    }

    //Update LEDs
    renderer.showPixels(); //Update the display (for hardware which is not instantaneous)
}

// Acceleration setter for simple 2D panel arrangements (for backwards compatibility with existing code)
void GravityParticles::setAcceleration(int16_t x, int16_t y)
{
    //Acceleration direction matches axes sign. 
    //e.g. +ve accelX will cause sand particles to move to greater X positions.
    accelX = x;
    accelY = y;
    
    //Limit maximum velocity based on strength of gravity
    uint16_t maxVel = sqrt( (int32_t)x*x+(int32_t)y*y ) * spaceMultiplier / 32;
    uint16_t minVelCap = spaceMultiplier / 4;
    // if (maxVel > minVelCap) {
    //     //Set max to 8 x magnitude of acceleration vector (set at pixel scale, so compensate for space mulipler)
    //     velCap = maxVel;
    // }
    // else {
    //     //Set to minimum
    //     velCap = minVelCap;
    // }

    char msg[255];
    sprintf(msg,"Acceleration set: %d,%d Vel min: %d, max: %d, cap: %d, shake=%d\n", accelX, accelY, minVelCap, maxVel, velCap, shake );
    renderer.outputMessage(msg);
}

// Acceleration setter for 3D panel arrangements (LED Cubes), sets an array of acceleration x,y components per panel
void GravityParticles::setAcceleration(int16_t x, int16_t y, int16_t z)
{
    //Acceleration direction matches axes sign. 
    //e.g. +ve accelX will cause sand particles to move to greater X positions.
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

void GravityParticles::addParticle(RGB_colour colour, int16_t vx, int16_t vy)
{
    //Place particle in random free position.
    //id indicates particle colour (currently based on 6 values each per r,g,b channel
    //so values from 1-215 represent colours)
    uint16_t x,y;
    uint16_t attempts = 0;
    do {
        x = renderer.random_int16(0,renderer.getGridWidth() ); // Assign random position within
        y = renderer.random_int16(0,renderer.getGridHeight() ); // the 'particle' coordinate space
        attempts++;
        // Check if corresponding pixel position is already occupied...
        // char msg[50];
        // sprintf(msg, "Random place attempt %d\n", attempts);
        // renderer.outputMessage(msg);
    } while ( (renderer.getPixelValue(y * renderer.getGridWidth() + x)) && (attempts < 2001) ); // Keep retrying until a clear spot is found
    
    //Add particle if free position was found
    if ( (renderer.getPixelValue(y * renderer.getGridWidth() + x)) == false ) {
        addParticle(x,y,colour,vx,vy);
    }
    else {
        char msg[50];
        sprintf(msg, "Failed to find free position for new particle.\n");
        renderer.outputMessage(msg);
    }

}

void GravityParticles::addParticle(uint16_t x, uint16_t y, RGB_colour colour, int16_t vx, int16_t vy)
{
    //Place particle into array at specified position.
    uint16_t i = numParticles;

    //Check for particles array overflow
    if (i == maxParticles) {
        //Expand particles array
        Particle* newArr = new Particle[maxParticles + 20];
        for (uint16_t i = 0; i < maxParticles; i++) {
            newArr[i] = particles[i];
        }
        delete[] particles;
        particles = newArr;
        maxParticles += 20;
        char msg[50];
        sprintf(msg, "Particle store expanded to size %d\n", maxParticles);
        renderer.outputMessage(msg);
    }

    particles[i].x = (x * spaceMultiplier)+renderer.random_int16(0,spaceMultiplier); // Assign position in centre of
    particles[i].y = (y * spaceMultiplier)+renderer.random_int16(0,spaceMultiplier); // the 'particle' coordinate space
    numParticles++;
    //Set initial velocity
    particles[i].vx = vx;
    particles[i].vy = vy; 
    renderer.setPixelValue( (particles[i].y / spaceMultiplier) * renderer.getGridWidth() + (particles[i].x / spaceMultiplier), renderer.getColourId(colour) ); // Mark it

// char msg[100];
// sprintf(msg, "Particle placed %d,%d (%d,%d) vel: %d,%d colour:%d; Total:%d\n", x,y, int(particles[i].x),int(particles[i].y), vx,vy, renderer.getColourId(colour), numParticles );
// renderer.outputMessage(msg);

}

GravityParticles::Particle GravityParticles::deleteParticle(uint16_t index)
{
    Particle particle;
    particle.x = particles[index].x;
    particle.y = particles[index].y;
    particle.vx = particles[index].vx;
    particle.vy = particles[index].vy;

    //Move all particles from specified index to end of array down one position
    for(uint16_t i=index; i<numParticles-1; i++) {
        particles[i].x = particles[i+1].x;
        particles[i].y = particles[i+1].y;
        particles[i].vx = particles[i+1].vx;
        particles[i].vy = particles[i+1].vy;
    }
    numParticles--;

    //Delete pixel where old particle was
    renderer.setPixelValue( (particle.y / spaceMultiplier) * renderer.getGridWidth() + (particle.x / spaceMultiplier), 0 ); // Mark it
    renderer.setPixelInstant(particle.x/spaceMultiplier,particle.y/spaceMultiplier, renderer.getColour(0) );

    return particle;
}

GravityParticles::Particle GravityParticles::getParticle(uint16_t index)
{
    Particle particle;

    //Convert position back into pixel coordinates
    particle.x = particles[index].x/spaceMultiplier;
    particle.y = particles[index].y/spaceMultiplier;
    particle.vx = particles[index].vx;
    particle.vy = particles[index].vy;

    return particle;
}

uint16_t GravityParticles::getParticleCount()
{
    return numParticles;
}

void GravityParticles::clearParticles()
{
    //Simply set number of particles to zero
    numParticles = 0;
}

void GravityParticles::imgToParticles()
{
    //Convert all pixels in current image to particles
    for(int y=0; y<renderer.getGridHeight(); y++) {
        for(int x=0; x<renderer.getGridWidth(); x++) {
            uint8_t colcode = renderer.getPixelValue(x,y);
            if (colcode > 0) {
                addParticle(x,y,renderer.getColour(colcode));
            }
        }
    }

}