/**************************************************************************************************
 * Conway's Game of Life 
 * 
 * This implementation was written as a reusable animator class where the RGB matrix hardware
 * rendering class is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on the hardware.
 *
 * Based on code which was originally published in my Ardunio code examples repo:
 * https://github.com/Footleg/arduino
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

/* NOTE: This code resets the simulation when repeating end conditions occur.
         The code detects repeating patterns of up to 24 cycles.
		 The reason each simulation is terminated is output to stderr.
*/

#include "golife.h"

// default constructor
GameOfLife::GameOfLife(RGBMatrixRenderer &renderer_, uint8_t fadeSteps_, uint16_t delay_, uint8_t startPattern_, uint8_t patternRepeatX_, uint8_t patternRepeatY_)
    : renderer(renderer_)
{
    // Allocate memory
    cells = new uint8_t*[renderer.getGridWidth()];
    for (uint16_t x=0; x<renderer.getGridWidth(); ++x) {
        cells[x] = new uint8_t[renderer.getGridHeight()];
    }

    //Initialise member variables
    fadeSteps = fadeSteps_;
    fadeStep = fadeSteps;
    delayms = delay_;
    startOver = true;
    fadeOn = false;
    startPattern = startPattern_;
    
    if (patternRepeatX_ < 1) {
        patternRepeatX = 1;
    }
    else {
        patternRepeatX = patternRepeatX_;
    }
    if (patternRepeatY_ < 1) {
        patternRepeatY = 1;
    }
    else {
        patternRepeatY = patternRepeatY_;
    }

    cellColours = new RGB_colour[8];


    panelSize = renderer.getGridHeight();
    if (renderer.getGridWidth() < panelSize)
        panelSize = renderer.getGridWidth();

} //GameOfLife

// default destructor
GameOfLife::~GameOfLife()
{
    for (uint16_t x=0; x<renderer.getGridWidth(); ++x) {
        delete [] cells[x];
    }
    delete [] cells;
    delete [] cellColours;
} //~GameOfLife

void GameOfLife::runCycle()
{
    int16_t x, y, xt, yt, xi, yi, neighbours;
    uint8_t maxRepeatsCount, maxContributor;

    //Get highest repeating frame count for repeating patterns > 5 frames
    maxRepeatsCount = 0;
    maxContributor = 0;
    for(int8_t i = 4; i < maxRepeatCycle; ++i)
    {
        if ( unchangedPopulation[i] > maxRepeatsCount )
        {
            maxRepeatsCount = unchangedPopulation[i];
            maxContributor = i;
        }
    }
    
    /* Reinitialise simulation on the following conditions:
    *  - All cells dead.
    *  - No changes have occurred between consecutive frames (static pattern)
    *  - Pattern alternates between just 2 different states 
    *  - Pattern cycles between 3 different states 
    *  - Population remains constant at 5 cells for 4xPanel size consecutive frames (gliding pattern)
    *  - Population remains constant at >5 cells 10xPanel size frames (as glider may collide with something)
    *  - Population cycles over a 4 step cycle for over 3xPanel size frames
    *  - Pattern cycles over 6-24 frames for over 200 cycles
    */
    if ( startOver || (alive == 0) || (unchangedCount > 5 ) || (repeat2Count > 6) || (repeat3Count > 35) 
        || (unchangedPopulation[0] > panelSize*10) || ( (unchangedPopulation[0] > panelSize*4) && (alive == 5 ) ) 
        || (unchangedPopulation[3] > panelSize*3) || (maxRepeatsCount > 200) )
    {
        //Update min and max iterations counters
        if (iterations > 0)
        {
            if (iterations < iterationsMin) iterationsMin = iterations;
            if (iterations > iterationsMax) iterationsMax = iterations;

        }
        //Debug end condition detected
        char msgEnd[80];
        if (alive == 0) 
            sprintf(msgEnd, "All died\n");
        else if (unchangedCount > 5 )
            sprintf(msgEnd, "Static pattern for 5 frames\n");
        else if (repeat2Count > 6)
            sprintf(msgEnd, "Pattern repeated over 2 frames\n");
        else if (repeat3Count > 35)
            sprintf(msgEnd, "Pattern repeated over 3 frames\n");
        else if (unchangedPopulation[0] > panelSize*10)
        {
            sprintf(msgEnd, "Population static over %d frames\n", (panelSize*10) );
        }
        else if ( (unchangedPopulation[0] > panelSize*4) && (alive == 5 ) )
        {
            sprintf(msgEnd, "Population static over %d frames with 5 cells exactly\n", (panelSize*4) );
        }
        else if (unchangedPopulation[3] > panelSize*3) 
        {
            sprintf(msgEnd, "Population repeated over 4 step cycle %d x\n", (panelSize*3) );
        }
        else if (maxRepeatsCount > 150)
        {
            sprintf(msgEnd, "Population repeated over %d step cycle 150x\n", (maxContributor+1) );
        }
        
        char msg[255];
        sprintf(msg, "Pattern terminated after %lu iterations (min: %lu, max: %lu): %s",
                (unsigned long)iterations, (unsigned long)iterationsMin, (unsigned long)iterationsMax, msgEnd);
        if (iterations > 0)
            renderer.outputMessage(msg);

        initialiseGrid(startPattern);
        
    }
    else if (fadeOn) {
        fadeStep++;
        fadeInChanges(fadeStep);
        if (fadeStep >= fadeSteps) {
            //End of fade, so update display
            fadeOn = false;
            renderer.msSleep(delayms);
            applyChanges();
            renderer.updateDisplay();
        }
    }
    else {
        //Run and update cycle
        // if ((delayms < 5) && ( (alive == 0) || (unchangedCount > 5 ) || (repeat2Count > 6) || (repeat3Count > 10) 
        //     || (unchangedPopulation[0] > 10)  
        //     || (unchangedPopulation[3] > 10) || (maxRepeatsCount > 20) ) )
        // {
        //     //Debug delay
        //     renderer.msSleep(100);
        // }


        //Apply rules of Game of Life to determine cells dying and being born
        for(y = 0; y < renderer.getGridHeight(); ++y)
        {
            for(x = 0; x < renderer.getGridWidth(); ++x)
            {
                //For each cell, count neighbours, including wrapping over grid edges
                neighbours = -1;
                uint8_t scores[8] = {0,0,0,0,0,0,0,0}; //To count colours of surrounding cells to decide colour of new cell
                for(xi = -1; xi < 2; ++xi)
                {
                    xt = renderer.newPositionX(x, xi);
                    for(yi = -1; yi < 2; ++yi)
                    {
                        yt = renderer.newPositionY(y, yi);
                        if ( (cells[xt][yt] & CELL_ALIVE) != 0 )
                        {
                            ++neighbours;
                            uint8_t colIdx = cells[xt][yt] >> 5;
                            scores[colIdx]++;
                        }
                    }
                }

                /* 
                    00: Cell is empty (free space) and staying that way
                    01: Cell is alive and surviving the next cycle
                    11: Cell is alive but is dying (counts as populated for this cycle but will be kills at end of cycle)
                    10: Cell is empty, but is going to spawn a new cell on the next cycle
                */
                //Initialise this cell change state to false
                cells[x][y] &= ~CELL_CHANGE;
                if ( ((cells[x][y] & CELL_ALIVE) != 0) && (neighbours < 2) )
                {
                    //Populated cell with too few neighbours, so it will die
                    cells[x][y] |= CELL_CHANGE; //turn on change bit

                }
                else if ( ((cells[x][y] & CELL_ALIVE) == 0) && (neighbours == 2) ) 
                {
                    //Empty cell with exactly 3 neighbours (count = 2 as did not count itself so was initialised as -1), so spawn new cell
                    cells[x][y] |= CELL_CHANGE; //turn on change bit
                    //Determine highest scoring colour from neighbours
                    uint8_t maxScore = 0;
                    uint8_t newCol = 0;
                    for(uint8_t i = 0; i < 8; ++i) {
                        if (scores[i] > maxScore) {
                            maxScore = scores[i];
                            newCol = i;
                        }
                    }

                    //Set new cell colour
                    cells[x][y] &= ~0b11100000;//Clear all colour bits
                    //Apply new colour bit values
                    cells[x][y] += newCol << 5;
// const char *bit_rep[16] = {
//     [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
//     [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
//     [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
//     [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
// };
// uint8_t byte = cells[x][y];
// sprintf(msg, "New cell value: %s%s.\n", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
// renderer.outputMessage(msg);

                }
                else if ( ((cells[x][y] & CELL_ALIVE) != 0) && (neighbours > 3) )
                {
                    //Populated cell with too many neighbours, so it will die
                    cells[x][y] |= CELL_CHANGE; //turn on change bit
                }
            }
        }

        //Fade cells in/out for births/deaths if fade steps set
        if (fadeSteps > 1) {
            // Trigger fade steps mode by resetting steps counter
            fadeStep = 0;
            fadeOn = true;
        }
        else {
            applyChanges();
            renderer.updateDisplay();
        }

        if (alive == 0)
        {
            //Pause to show end of population before it gets reset
            uint16_t waitLength = delayms * 100;
            if (waitLength > 3000) waitLength = 3000;
            renderer.msSleep(waitLength); 
        }
    }

    renderer.msSleep(delayms);
    iterations++;
}

// Set index of preset pattern to start animation with
void GameOfLife::setStartPattern(uint8_t patternIdx)
{
    if ( (patternIdx > 0) && (patternIdx < 8) ) {
        startPattern = patternIdx;
    } 
    else {
        startPattern = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialise Grid
///////////////////////////////////////////////////////////////////////////////////////////////////
void GameOfLife::initialiseGrid(uint8_t patternIdx)
{
    const bool X = true;
    const bool O = false;

    //Wipe img to reset palette
    renderer.clearImage();

    alive = 0;
    iterations = 0;
    fadeOn = false;
    fadeStep = fadeSteps;
    unchangedCount = 0;
    for (uint8_t x = 0; x < maxRepeatCycle; ++x) unchangedPopulation[x] = 0;
    repeat2Count = 0;
    repeat3Count = 0;
    for (uint8_t x = 0; x < popHistorySize; ++x) population[x] = 0;

    //New random colours
    for (uint8_t i=0; i < 8; i++) {
        cellColours[i] = renderer.getRandomColour();
    
        if (fadeSteps > 4) {
            //Reject colours which are too close to red or green
            const uint8_t maxDiff = 80;
            while ( ( (cellColours[i].r - cellColours[i].g > maxDiff) && (cellColours[i].r - cellColours[i].b > maxDiff) ) 
                || ( (cellColours[i].g - cellColours[i].r > maxDiff) && (cellColours[i].g - cellColours[i].b > maxDiff) ) ) {

                char msg[32];
                sprintf(msg, "Rejected colour  %d, %d, %d\n", cellColours[i].r,  cellColours[i].g, cellColours[i].b);
                renderer.outputMessage(msg);
                cellColours[i] = renderer.getRandomColour();
                    
                }
        }
    }

    // //Debug using fixed colours
    // cellColours[0] = RGB_colour(255,0,0);
    // cellColours[1] = RGB_colour(255,100,0);
    // cellColours[2] = RGB_colour(255,255,0);
    // cellColours[3] = RGB_colour(0,255,0);
    // cellColours[4] = RGB_colour(0,255,255);
    // cellColours[5] = RGB_colour(0,0,255);
    // cellColours[6] = RGB_colour(255,0,255);
    // cellColours[7] = RGB_colour(100,0,255);

    if (patternIdx == 0) {
        //Random
        for(uint16_t y = 0; y < renderer.getGridHeight(); ++y)
        {
            for(uint16_t x = 0; x < renderer.getGridWidth(); ++x)
            {
                uint8_t randNumber = renderer.random_int16(0,100);
                if (randNumber < 15)
                {
                    //Pick random colour from palette
                    uint8_t colIdx = renderer.random_int16(0,8);
                    cells[x][y] = colIdx << 5; //Set colour bits (3 RH most bits)
                    cells[x][y] |= CELL_ALIVE; //Set cell alive bit
                    renderer.setPixelColour(x, y, cellColours[colIdx]);
                    alive++;
                }
                else
                {
                    cells[x][y] = 0;
                    renderer.setPixelColour(x, y, RGB_colour{0,0,0} );
                }
            }
        }
    }
    else
    {
        bool pattern[256] = {O};

        
        switch(patternIdx)
        {
            case 1:
            {
                bool patternAlt[] = {
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,
                O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,
                O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,X,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O };
                
                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;

            case 2:
            {
                bool patternAlt[] = {
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];                    
            }
            break;

            case 3:
            {
                bool patternAlt[] = {
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,X,X,O,X,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,
                O,O,O,O,O,O,X,X,O,X,O,O,O,O,O,O,
                O,O,O,O,O,X,O,X,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
            case 4:
            {
                bool patternAlt[] = {
                O,O,O,O,X,X,X,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
            case 5:
            {
                bool patternAlt[] = {
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
            case 6:
            {
                bool patternAlt[] = {
                O,O,O,O,X,X,X,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,X,X,O,O,O,O,O,O,X,
                O,O,O,O,O,O,X,O,O,X,O,O,O,X,O,X,
                O,O,O,O,O,X,O,O,O,O,X,O,O,O,X,X,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                O,O,O,O,X,O,O,O,O,O,O,X,O,O,O,O,
                X,X,O,O,O,X,O,O,O,O,X,O,O,O,O,O,
                X,O,X,O,O,O,X,O,O,X,O,O,O,O,O,O,
                X,O,O,O,O,O,O,X,X,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,X,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,X,X,X,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
            case 7:
            {
                bool patternAlt[] = {
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,X,X,X,O,O,O,O,O,
                O,O,O,O,O,X,X,X,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,X,X,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,X,X,O,O,O,O,O,O,O,O,
                O,O,O,O,O,X,O,O,X,X,X,O,O,O,O,O,
                O,O,O,O,O,X,X,X,O,O,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,X,X,X,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
            case 8:
            {
                bool patternAlt[] = {
                X,X,X,X,X,X,O,O,O,O,O,O,O,O,O,O,
                X,O,O,O,O,O,X,O,O,O,O,O,O,O,O,O,
                X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,X,O,O,O,O,X,O,O,O,O,O,O,X,X,X,
                O,O,O,X,X,O,O,O,O,O,O,O,O,O,O,X,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                O,X,O,O,X,O,O,O,O,O,O,O,O,O,O,O,
                X,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,
                X,O,O,O,X,O,O,O,O,O,O,O,O,O,O,O,
                X,X,X,X,O,O,O,O,O,O,O,O,O,O,O,O,
                O,O,O,O,O,O,O,O,O,O,O,O,X,O,O,O,
                O,O,O,O,O,O,O,O,O,O,X,O,O,O,X,O,
                O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,X,
                O,O,O,O,O,O,O,O,O,O,X,O,O,O,O,X,
                O,O,O,O,O,O,O,O,O,O,O,X,X,X,X,X };

                for (uint16_t i=0; i < 256; i++) pattern[i] = patternAlt[i];
            }
            break;
                
        }

        //Clear entire array of cells
        for(uint16_t y = 0; y < renderer.getGridHeight(); ++y)
        {
            for(uint16_t x = 0; x < renderer.getGridWidth(); ++x)
            { 
                //Clear cells outside pattern
                cells[x][y] &= ~CELL_ALIVE;
                renderer.setPixelColour(x, y, RGB_colour{0,0,0} );
            }
        }

        //Set up patterns
        uint8_t colIdx = 0;
        for(uint16_t py = 0; py < patternRepeatY; ++py) { 
            for(uint16_t px = 0; px < patternRepeatX; ++px) { 
                uint16_t spacingX = renderer.getGridWidth() / (patternRepeatX + 1);
                uint16_t spacingY = renderer.getGridHeight() / (patternRepeatY + 1);
                uint16_t offsetX = spacingX * (px + 1);
                uint16_t offsetY = spacingY * (py + 1);

                //Set pattern cells
                for(uint16_t y = offsetY; y < offsetY + 16; ++y) { 
                    for(uint16_t x = offsetX; x < offsetX + 16; ++x) { 
                        //Only set cells if inside display area
                        if ( (x < renderer.getGridWidth()) && (y < renderer.getGridHeight()) ) {
                            if ( (pattern[(16 - 1 - y+offsetY)*16 + x-offsetX]) ) {
                                cells[x][y] = colIdx << 5; //Set colour bits (3 RH most bits)
                                cells[x][y] |= CELL_ALIVE; //Set cell alive bit
                                renderer.setPixelColour(x, y, cellColours[colIdx]);
                                alive++;
                            }
                        }
                    }
                }

                //Increment colour from palette
                colIdx++;
                if (colIdx > 7) colIdx = 0;
            }
        }

    }

    renderer.updateDisplay();

    //Clear restart flag
    startOver = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update cells array with changes for next iteration
///////////////////////////////////////////////////////////////////////////////////////////////////
void GameOfLife::applyChanges()
{
    uint8_t changes, gap;
    int8_t popChk,prevPopChk;
    
    changes = 0;
    bool compare2 = true;
    bool compare3 = true;

    for(uint16_t y = 0; y < renderer.getGridHeight(); ++y)
    {
        for(uint16_t x = 0; x < renderer.getGridWidth(); ++x)
        {
            //Update last 3 iterations history for this cell
            if ((cells[x][y] & CELL_PREV2) != 0)
                cells[x][y] |= CELL_PREV3;
            else
                cells[x][y] &= ~CELL_PREV3;
            
            if ((cells[x][y] & CELL_PREV1) != 0)
                cells[x][y] |= CELL_PREV2;
            else
                cells[x][y] &= ~CELL_PREV2;

            if ((cells[x][y] & CELL_ALIVE) != 0)
                cells[x][y] |= CELL_PREV1;
            else
                cells[x][y] &= ~CELL_PREV1;

            //Create new cells
            if ( ((cells[x][y] & CELL_ALIVE) == 0) && ((cells[x][y] & CELL_CHANGE) != 0))
            {
                cells[x][y] |= CELL_ALIVE;
                uint8_t colIdx = cells[x][y] >> 5;
                renderer.setPixelColour(x, y, cellColours[colIdx]);
                ++changes;
                ++alive;
            }
            else if ( ((cells[x][y] & CELL_ALIVE) != 0) && ((cells[x][y] & CELL_CHANGE) != 0))
            {
                //Kill dying cells
                cells[x][y] &= ~CELL_ALIVE;
                renderer.setPixelColour(x, y, RGB_colour{0,0,0} );
                ++changes;
                --alive;
            }
            
            //Compare cell to state 2 and 3 iterations ago
            if (compare2 && ( ((cells[x][y] & CELL_ALIVE) == 0) != ((cells[x][y] & CELL_PREV2) == 0) )) compare2 = false;
            if (compare3 && ( ((cells[x][y] & CELL_ALIVE) == 0) != ((cells[x][y] & CELL_PREV3) == 0) )) compare3 = false;
        }
    }

    popCursor++;
    if (popCursor > popHistorySize-1) popCursor = 0;
    population[popCursor] = alive;
    
    //Increment counter if no changes made
    if (changes == 0)
        ++unchangedCount;
    else
        unchangedCount = 0;

    //Increment counter if last frame was identical to 2 frames ago (2 cycle repeat)
    if (compare2)
        ++repeat2Count;
    else
        repeat2Count = 0;

    //Increment counter if last frame was identical to 3 frames ago (3 cycle repeat)
    if (compare3)
        ++repeat3Count;
    else
        repeat3Count = 0;

    //Increment counter of unchanging population size
    if (popCursor == 0)
        popChk = popHistorySize-1;
    else
        popChk = popCursor - 1;

    if (population[popChk] == alive)
        ++unchangedPopulation[0];
    else
        unchangedPopulation[0] = 0;

    //Check for repeating population cycles
    bool gapCheck = false;
    for (gap = 4; gap < maxRepeatCycle+1; ++gap)
    {
        for (uint8_t i = 1; i < popHistorySize/gap; ++i)
        {
        
        for (uint8_t j = 0; j < gap; ++j)
        {
            popChk = popCursor - 1 - (gap * i) - j;
            if (popChk < 0) popChk += popHistorySize;
            prevPopChk = popChk + (gap * i);
            if (prevPopChk > popHistorySize-1) prevPopChk -= popHistorySize;
            gapCheck = ( (population[popChk] > 0) && (population[popChk] == population[prevPopChk]) );

            if (gapCheck == false)
            {
            break;
            }
        }
        if (gapCheck == false)
        {
            break;
        }
        }
        if (gapCheck == true) break;
    }

    if (gapCheck == true) ++unchangedPopulation[gap-1];
    else unchangedPopulation[gap-1] = 0;

// char msg[64];
// sprintf(msg, "Repeat 2 count: %d, Repeat 3 count: %d\n", repeat2Count, repeat3Count);
// renderer.outputMessage(msg);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Fade births in green, and death to red
///////////////////////////////////////////////////////////////////////////////////////////////////
void GameOfLife::fadeInChanges(uint8_t step)
{
    uint8_t halfSteps = fadeSteps / 2; 
    RGB_colour born;
    RGB_colour died;
    uint16_t maxBrightness = (uint16_t)(cellColours[0].r + cellColours[0].g + cellColours[0].b)/2;
    if (maxBrightness > 128) maxBrightness = 128;
    uint8_t max8bit = (uint8_t)(maxBrightness);

    if (step <= halfSteps) {
        //Set fade from black to green for cells being born
        born = renderer.blendColour(RGB_colour{0,0,0}, RGB_colour{0,max8bit,0}, step, halfSteps);
    } else {
        //Set fade colour for cells dying out from red to black
        died = renderer.blendColour(RGB_colour{max8bit,0,0}, RGB_colour{0,0,0}, step-halfSteps, fadeSteps-halfSteps);
    }
    
    for(uint16_t y = 0; y < renderer.getGridHeight(); ++y)
    {
        for(uint16_t x = 0; x < renderer.getGridWidth(); ++x)
        {
            uint8_t colIdx = cells[x][y] >> 5;


            if ( ((cells[x][y] & CELL_ALIVE) == 0) && ((cells[x][y] & CELL_CHANGE) != 0))
            {
                if (step <= halfSteps) {
                    renderer.setPixelInstant(x, y, born);
                } else {
                    //Set fade from green to active cell colour for cells being born
                    born = renderer.blendColour(RGB_colour{0,max8bit,0}, cellColours[colIdx], step-halfSteps, fadeSteps-halfSteps);
                    renderer.setPixelInstant(x, y, born);
                }
            }
            else if ( ((cells[x][y] & CELL_ALIVE) != 0) && ((cells[x][y] & CELL_CHANGE) != 0))
            {
                if (step <= halfSteps) {
                    //Set fade cells dying out from current colour to red
                    died = renderer.blendColour(cellColours[colIdx], RGB_colour{max8bit,0,0}, step, halfSteps);
                }
                renderer.setPixelInstant(x, y, died);
            }
            else if ((cells[x][y] & CELL_ALIVE) != 0)
            {
                renderer.setPixelInstant(x, y, cellColours[colIdx]);
            }
        }
    }
/*
char msg[100];
sprintf(msg, "Born col: %d,%d,%d Dead col: %d,%d,%d\n", born.r, born.g, born.b, died.r, died.g, died.b );
renderer.outputMessage(msg);
*/      

    renderer.showPixels();
}

bool GameOfLife::getCellState(uint16_t x, uint16_t y)
{
    return ( (cells[x][y] & CELL_ALIVE) != 0 );
}

RGB_colour GameOfLife::getCellColour(uint8_t idx)
{
    return cellColours[idx];
}

void GameOfLife::restart()
{
    startOver = true;
}