/**************************************************************************************************
 * Simple Crawler animator class 
 * 
 * Generates a simple animation starting at a random point in the grid, and crawling across the
 * grid at random. This is a simple example of a reusable animator class where the RGB matrix
 * hardware rendering class is passed in, so it can be used with any RGB array display by writing 
 * an implementation of a renderer class to set pixels/LED colours on the hardware.
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

#include "crawler.h"

// default constructor
Crawler::Crawler(RGBMatrixRenderer &renderer_, uint16_t steps)
    : renderer(renderer_), dirChgCount(steps)
{
    //Pick random start point
    x = rand()%renderer.getGridWidth();
    y = rand()%renderer.getGridHeight();

    //Start random direction
    direction = rand()%4;

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
    renderer.setPixelColour(x, y, colour);

    //Clear pixels around direction of travel
    switch(direction) {
        case 0: //Up
            renderer.setPixelColour(renderer.newPositionX(x,-1,false), y, RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1,false), y, RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,-1), renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1), renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(x, renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            break;
        case 1: //Right
            renderer.setPixelColour(x, renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(x, renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1), renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1), renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1), y, RGB_colour{0, 0, 0} );
            break;
        case 2: //Down
            renderer.setPixelColour(renderer.newPositionX(x,-1,false), y, RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1,false), y, RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,-1), renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,1), renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(x, renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            break;
        case 3: //Left
            renderer.setPixelColour(x, renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(x, renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,-1), renderer.newPositionY(y,-1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,-1), renderer.newPositionY(y,1), RGB_colour{0, 0, 0} );
            renderer.setPixelColour(renderer.newPositionX(x,-1), y, RGB_colour{0, 0, 0} );
            break;
    }
    
    renderer.updateDisplay();
    
    //Update direction if more than 1 step since last change
    dirChg++;
    if (dirChg > 1) {
        int c = rand()%8;
        switch(c) {
            case 0: //Turn left
                direction--;
                dirChg = 0;
                break;
            case 1: //Turn right
                direction++;
                dirChg = 0;
                break;

        }
        if (direction > 3)
            direction = 0;
        else if (direction < 0)
            direction = 3;
    }

    //Update postion
    switch(direction) {
        case 0: //Up
            y = renderer.newPositionY(y,1);
            break;
        case 1: //Right
            x = renderer.newPositionX(x,1);
            break;
        case 2: //Down
            y = renderer.newPositionY(y,-1);
            break;
        case 3: //Left
            x = renderer.newPositionX(x,-1);
            break;
    }
    //fprintf(stderr, "%s %d %s %d %s", "Pos: ", x, ",",  y, "\n" );

    //Update colour every x steps
    colChg++;
    if (colChg >= dirChgCount) {
        colChg = 0;
        colour = renderer.getRandomColour();
    }

}