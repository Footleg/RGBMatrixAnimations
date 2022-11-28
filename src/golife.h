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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(ARDUINO)
#include "WProgram.h"
#else
#include <stdio.h>
#endif

#include "RGBMatrixRenderer.h"

class GameOfLife
{
    //variables
    public:
    protected:
    private:
        static uint8_t const maxRepeatCycle = 24;
        static uint8_t const popHistorySize = 48;
        static uint8_t const CELL_ALIVE = 0b00000001;
        static uint8_t const CELL_CHANGE = 0b00000010;
        static uint8_t const CELL_PREV1 = 0b00000100;
        static uint8_t const CELL_PREV2 = 0b00001000;
        static uint8_t const CELL_PREV3 = 0b00010000;
        //static uint8_t const CELL_COL1 = 0b00100000;
        RGB_colour* cellColours;
        uint16_t delayms;
        uint8_t fadeSteps;
        uint8_t fadeStep = 1;
        uint8_t startPattern = 0;
        uint8_t patternRepeatX = 1;
        uint8_t patternRepeatY = 1;
        RGBMatrixRenderer &renderer;
        uint8_t** cells; //8bits representing [colour3,colour2,colour1,prev3,prev2,prev1,birth/death,alive]
        uint16_t alive = 0;
        uint16_t population[popHistorySize] = {};
        uint8_t popCursor = popHistorySize - 1; //Set to last position as gets incremented before use
        uint8_t unchangedCount = 0;
        uint8_t repeat2Count = 0;
        uint8_t repeat3Count = 0;
        uint8_t unchangedPopulation[maxRepeatCycle] = {};
        uint32_t iterations = 0;
        uint32_t iterationsMin = 4294967295;
        uint32_t iterationsMax = 0;
        uint16_t panelSize;
        bool startOver;
        bool fadeOn;
    //functions
    public:
        GameOfLife(RGBMatrixRenderer&,uint8_t,uint16_t,uint8_t,uint8_t=1,uint8_t=1);
        ~GameOfLife();
        void runCycle();
        void restart();
        bool getCellState(uint16_t,uint16_t);
        void setStartPattern(uint8_t);
        RGB_colour getCellColour(uint8_t);
    protected:
    private:
        void initialiseGrid(uint8_t);
        void applyChanges();
        void fadeInChanges(uint8_t);
}; //GameOfLife
