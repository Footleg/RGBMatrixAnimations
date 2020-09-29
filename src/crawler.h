/**************************************************************************************************
 * Simple Crawler animator class 
 * 
 * Generates a simple animation starting at a random point in the grid, and crawling across the
 * grid at random. This is a simple example of an animator class where the RGB matrix hardware
 * rendering is passed in, so it can be used with any RGB array display by writing an 
 * implementation of a renderer class to set pixels/LED colours on the hardware.
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

#include "RGBMatrixRenderer.h"

class Crawler
{
    //variables
    public:
    protected:
    private:
        RGBMatrixRenderer &renderer;
        int x;
        int y;
        int direction;
        int colChg = 0;
        int dirChg = 0;
    //functions
    public:
        Crawler(RGBMatrixRenderer&);
        ~Crawler();
        void runCycle();
    protected:
    private:

}; //Crawler