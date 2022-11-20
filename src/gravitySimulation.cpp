/**************************************************************************************************
 * Gravity simulation
 * 
 * This implementation was written as a reusable animator class where the RGB matrix hardware
 * rendering class is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on your specific hardware.
 *
 * This class simulates spheres in a 2-D space with attractive or repelling forces.
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


#include "gravitySimulation.h"  

// default constructor
GravitySimulation::GravitySimulation(RGBMatrixRenderer &renderer_, uint8_t maxRadius_)
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

    // The 'sand' balls exist in an integer coordinate space that's 256X
    // the scale of the pixel grid, allowing them to move and interact at
    // less than whole-pixel increments.
    maxX = renderer.getGridWidth() - 1; // Maximum X coordinate in particle space
    maxY = renderer.getGridHeight() - 1; // Maximum Y coordinate in particle space

    numBalls = 0;
    maxRadius = maxRadius_;

} //GravitySimulation

// default destructor
GravitySimulation::~GravitySimulation()
{
} //~GravitySimulation

GravitySimulation::Ball GravitySimulation::createBall() {
  GravitySimulation::Ball shape;
  shape.x = rand() % renderer.getGridWidth();
  shape.y = rand() % renderer.getGridHeight();
  if (maxRadius > 1){
    shape.r = (rand() % (maxRadius-1)) + 1;
  }
  else {
    shape.r = 1;
  }
  shape.dx = float(rand() % 255) / 64.0f;
  shape.dy = float(rand() % 255) / 64.0f;
  // Generate random colour which is not too dark
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  while (r + g + b < 192) {
    r = rand() % 255;
    g = rand() % 255;
    b = rand() % 255;
  }
  shape.colour = RGB_colour(r, g, b);
  return shape;
};

void GravitySimulation::addBall()
{
    shapes.push_back( createBall() );
}

//Run Cycle is called once per frame of the animation
void GravitySimulation::runCycle()
{
    uint16_t i = 0;
    uint16_t iterationsPerFrame = 1;
    // uint16_t ballCount = 0;
    for (u_int16_t iter = 0; iter < iterationsPerFrame; iter++){
        // ballCount = 0;
        for (auto &shape : shapes) {
            // ballCount++;
            // char msg[80];
            // sprintf(msg, "Balls processed %d\n", ballCount );
            // renderer.outputMessage(msg);

            float oldX = shape.x;
            float oldY = shape.y;

            //Update shape position
            shape.x += shape.dx/iterationsPerFrame;
            shape.y += shape.dy/iterationsPerFrame;

            //Check for collision with shapes already updated
            for(uint16_t j = 0; j < i; j++) {
                //Check distance between shapes
                float sepx = shapes[j].x - shape.x;
                float sepy = shapes[j].y - shape.y;
                uint16_t sep = int(sqrt((sepx*sepx)+(sepy*sepy)));

                //Don't try to process interactions if shapes exactly on top of one another
                if(sep > 0.0) {

                    float ax = 0.0f;
                    float ay = 0.0f;

                    uint8_t rd = 0;
                    float force = -forcePower;

                    //Bounce if contacting
                    rd = shape.r + shapes[j].r;
                    if( sep < rd) {
                        // If forces between balls, allow to pass each other when overlapping,
                        // unless centres really close. Don't apply forces during overlap as
                        // force is too strong and they just stick together.
                        if (mode == 0 || sep < rd / 4) {
                            //Bounce balls off each other
                            /*
                            //Trig solution
                            float angle = atan2(sepy, sepx);
                            float targetX = shape.x + cos(angle) * sep;
                            float targetY = shape.y + sin(angle) * sep;
                            float ax = (targetX - shape.x);
                            float ay = (targetY - shape.y);
                            */

                            //Handle x and y components seperately (more efficient)
                            ax = sepx;
                            ay = sepy;
                        }
                    }
                    else {
                        switch(mode){
                            case 1:
                            //Repel, Force is inverse of distance squared
                            force = force / (sep*sep);
                            ax = force * sepx / sep;
                            ay = force * sepy / sep;

                            break;
                        }
                    }

                    float prePower = sqrt(shape.dx*shape.dx+shape.dy*shape.dy) + sqrt(shapes[j].dx*shapes[j].dx+shapes[j].dy*shapes[j].dy);
                    shape.dx -= ax * shapes[j].r;
                    shape.dy -= ay * shapes[j].r;
                    shapes[j].dx += ax * shape.r;
                    shapes[j].dy += ay * shape.r;
                    float postPower = sqrt(shape.dx*shape.dx+shape.dy*shape.dy) + sqrt(shapes[j].dx*shapes[j].dx+shapes[j].dy*shapes[j].dy);
                    float scalePower = prePower / postPower;

                    shape.dx = shape.dx * scalePower;
                    shape.dy = shape.dy * scalePower;
                    shapes[j].dx = shapes[j].dx * scalePower;
                    shapes[j].dy = shapes[j].dy * scalePower;

                
                }
            }

            //Check shape remains in bounds of screen, reverse direction if not
            if((shape.x - shape.r) < minX) {
                shape.dx *= -1;
                shape.x = minX + shape.r;
            }
            if((shape.x + shape.r) >= maxX) {
                shape.dx *= -1;
                shape.x = maxX - shape.r;
            }
            if((shape.y - shape.r) < minY) {
                shape.dy *= -1;
                shape.y = minY + shape.r;
            }
            if((shape.y + shape.r) >= maxY) {
                shape.dy *= -1;
                shape.y = maxY - shape.r;
            }

            //Clear pixels for last drawn position of this circle on first iteration of positions simulation
            if(iter == 0){
                //Skip the slow calcs if 1:1 scale with screen
                if(minX == 0 && minY == 0){
                    //Draw circles at 1:1 scale on screen
                    //graphics.circle(Point(shape.x, shape.y), shape.r);
                    renderer.drawCircle(oldX, oldY, shape.r, RGB_colour(0,0,0)); 
                }
            }

            //Draw new position on last iteration
            if(iter == iterationsPerFrame-1){
                //Skip the slow calcs if 1:1 scale with screen
                if(minX == 0 && minY == 0){
                    //Draw circles at 1:1 scale on screen
                    //graphics.circle(Point(shape.x, shape.y), shape.r);
                    renderer.drawCircle(oldX, oldY, shape.r, RGB_colour(0,0,0)); //Clear old position
                    renderer.drawCircle(shape.x, shape.y, shape.r, shape.colour); //Draw in new position
                }
                else {
                    // //Draw circles scaled to boundaries
                    // float posX = graphics.bounds.w * (shape.x - minX) / (maxX - minX);
                    // float posY = graphics.bounds.h * (shape.y - minY) / (maxY - minY);
                    // float rad = graphics.bounds.h * shape.r / (maxY - minY);
                    // if(rad < 2) rad = 2;
                    // graphics.set_pen(shape.pen);
                    // graphics.circle(Point(posX, posY), rad);

                    
                }
            }

            i++;
        }

    }
 

    //Update LEDs
    renderer.showPixels(); //Update the display (for hardware which is not instantaneous)
}

void GravitySimulation::setMode(uint8_t mode_){
    mode = mode_;
}
