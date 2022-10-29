/**************************************************************************************************
 * Simple Crawler animator class 
 * 
 * Generates a simple animation starting at a random point in the grid, and crawling across the
 * grid at random. This is a simple example of a reusable animator class where the RGB matrix
 * hardware rendering class is passed in, so it can be used with any RGB array display by writing 
 * an implementation of a renderer class to set pixels/LED colours on the hardware.
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

#include "crawler.h"

// default constructor
Crawler::Crawler(RGBMatrixRenderer &renderer_, uint16_t steps, uint16_t minSteps, bool anyAngle)
    : renderer(renderer_), leadPixel(0,0,0,0), colChgCount(steps), dirChgCount(minSteps), anyAngle(anyAngle)
{
    //Pick random start point
    leadPixel.x = rand()%renderer.getGridWidth();
    leadPixel.y = rand()%renderer.getGridHeight();

    //Force random direction change on start
    dirChg = minSteps + 1;

    //Debugging, hard coded position and velocity
    // leadPixel.x = 10;
    // leadPixel.y = 3;
    // leadPixel.vx = -100;
    // leadPixel.vy = 0;

    //Initial random colour
    colour = renderer.getRandomColour();
} //Crawler

// default destructor
Crawler::~Crawler()
{
} //~Crawler

//Run Cycle is called once per frame of the animation
void Crawler::runCycle()
{
    //Example of how to output messages to console, compatible with Arduino and C++ on Linux
/*
    char msg[44];
    sprintf(msg, "%s %d %s %d %s %d %s", "Drawing colour: ", colour.r, ",",  colour.g,  ",",  colour.b, "\n" );
    renderer.outputMessage(msg);
*/    
    
    //Set current position pixel
    renderer.setPixelColour(leadPixel.x, leadPixel.y, colour);

    if (anyAngle==false){
        //Clear pixels around direction of travel
        if (leadPixel.vy == renderer.SUBPIXEL_RES) {
            //Up
            MovingPixel cursor(leadPixel.x,leadPixel.y,-renderer.SUBPIXEL_RES,0);
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = -renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );

        } 
        else if (leadPixel.vx == renderer.SUBPIXEL_RES)
        {
            //Right
            MovingPixel cursor(leadPixel.x,leadPixel.y,0,renderer.SUBPIXEL_RES);
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = -renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = -renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
    } 
        else if (leadPixel.vy == -renderer.SUBPIXEL_RES)
        {
            //Down
            MovingPixel cursor(leadPixel.x,leadPixel.y,-renderer.SUBPIXEL_RES,0);
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = -renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
        } 
        else 
        {
            //Left
            MovingPixel cursor(leadPixel.x,leadPixel.y,0,renderer.SUBPIXEL_RES);
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = -renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = 0;
            cursor.vy = -renderer.SUBPIXEL_RES;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
            cursor.vx = renderer.SUBPIXEL_RES;
            cursor.vy = 0;
            cursor = renderer.updatePosition(cursor);
            renderer.setPixelColour(cursor.x, cursor.y, RGB_colour{0, 0, 0} );
        }
    }
    
    renderer.updateDisplay();
    
    //Update direction if more than set number of steps since last change
    dirChg++;
    if (dirChg > dirChgCount) {
        dirChg = 0;
        
        // 2 out of 8 chance we change direction
        // 0 or 1 mean opposite directions to turn from current direction
        // 2 or above means keep going in current direction
        int c = rand()%8;
        int dir = 0;
        switch(c) {
            case 0: 
                dir = -1;
               break;
            case 1: 
                dir = 1;
                break;
        }

        //Override random change if no velocity (happens on start-up)
        if (leadPixel.vx == 0 && leadPixel.vy == 0) {
            dir = 1;
        }

        if (dir != 0 ) {
            if (leadPixel.vx == 0) {
                if (anyAngle) {
                    leadPixel.vx = dir*rand()%renderer.SUBPIXEL_RES+renderer.SUBPIXEL_RES;
                    leadPixel.vy = dir*rand()%renderer.SUBPIXEL_RES;
                }
                else {
                    leadPixel.vx = renderer.SUBPIXEL_RES;
                    leadPixel.vy = 0;
                }
            }
            else {
                if (anyAngle) {
                    leadPixel.vx = dir*rand()%renderer.SUBPIXEL_RES;
                    leadPixel.vy = dir*rand()%renderer.SUBPIXEL_RES+renderer.SUBPIXEL_RES;
                }
                else {
                    leadPixel.vx = 0;
                    leadPixel.vy = renderer.SUBPIXEL_RES;
                }
                
            }
        }
    }

    //Update postion
    leadPixel = renderer.updatePosition(leadPixel);

    char msg[64];
    sprintf(msg, "New pos: %d,%d Vel: %d,%d\n", leadPixel.x, leadPixel.y, leadPixel.vx, leadPixel.vy );
    renderer.outputMessage(msg);

    //Update colour every x steps
    colChg++;
    if (colChg >= colChgCount) {
        colChg = 0;
        colour = renderer.getRandomColour();
    }

}