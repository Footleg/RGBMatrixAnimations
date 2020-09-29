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
        static uint8_t const CELL_ALIVE = 0x01;
        static uint8_t const CELL_PREV1 = 0x02;
        static uint8_t const CELL_PREV2 = 0x04;
        static uint8_t const CELL_PREV3 = 0x08;
        static uint8_t const CELL_BIRTH = 0x10;
        static uint8_t const CELL_DEATH = 0x20;
        int delayms;
        uint8_t fadeSteps;
        RGBMatrixRenderer &renderer;
        uint8_t** cells; //8bits representing [null,null,deaths,births,prev3,prev2,prev1,alive]
        uint8_t alive = 0;
        uint8_t population[popHistorySize] = {};
        uint8_t popCursor = popHistorySize - 1; //Set to last position as gets incremented before use
        uint8_t unchangedCount = 0;
        uint8_t repeat2Count = 0;
        uint8_t repeat3Count = 0;
        uint8_t unchangedPopulation[maxRepeatCycle] = {};
        uint32_t iterations = 0;
        uint32_t iterationsMin = 4294967295;
        uint32_t iterationsMax = 0;
        int panelSize;
    //functions
    public:
        GameOfLife(RGBMatrixRenderer&,uint8_t,int);
        ~GameOfLife();
        void runCycle();
    protected:
    private:
        void initialiseGrid(uint8_t);
        void applyChanges();
        void fadeInChanges();
}; //GameOfLife
