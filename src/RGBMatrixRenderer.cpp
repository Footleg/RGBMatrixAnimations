/**************************************************************************************************
 * RGB Matrix Renderer abstract class 
 * 
 * This abstract class provides a template for building renderer classes to control LED Matrix
 * displays. It provides methods to set the colour of any LED/pixel in a grid, and to keep the
 * position of points within the bound of the grid (with or without wrapping over the edges).
 * A series of animation classes use the interface defined in this class to output to displays.
 * By writing a renderer class for any LED matrix hardware (or on screen display) this allows the
 * animations classes to be used with many different hardware display devices without requiring
 * code changes in the animation class.
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

// default constructor
RGBMatrixRenderer::RGBMatrixRenderer(uint16_t width, uint16_t height, uint8_t maxBrightness)
    : gridWidth(width), gridHeight(height), maxBrightness(maxBrightness)
{} //RGBMatrixRenderer

// default destructor
RGBMatrixRenderer::~RGBMatrixRenderer()
{
} //~RGBMatrixRenderer

uint16_t RGBMatrixRenderer::getGridWidth()
{
    return gridWidth;
}

uint16_t RGBMatrixRenderer::getGridHeight()
{
    return gridHeight;
}

uint8_t RGBMatrixRenderer::getMaxBrightness()
{
    return maxBrightness;
}

void RGBMatrixRenderer::setRandomColour()
{
    // Init colour randomly
    r = random_int16(0,maxBrightness);
    g = random_int16(0,maxBrightness);
    b = random_int16(0,maxBrightness);
    uint8_t minBrightness = maxBrightness * 3 / 4;
    
    //Prevent colours being too dim
    if (r<minBrightness && g<minBrightness && b<minBrightness) {
        uint8_t c = random_int16(0,3);
        switch (c) {
        case 0:
            r = 200;
            break;
        case 1:
            g = 200;
            break;
        case 2:
            b = 200;
            break;
        }
    }
    
    //This method for string concatenation was used as it compiles on both g++ on Linux
    //and arduino platforms
    char msg[32];
    sprintf(msg, "New RGB colour  %d, %d, %d\n", r,  g, b);
    outputMessage(msg);

}

//Method to update a grid coordinate while keeping on the matrix, with optional wrapping
uint16_t RGBMatrixRenderer::newPosition(uint16_t position, uint16_t increment, uint16_t dimension, bool wrap)
{
    int16_t newPos = position + increment;

    if (wrap) 
    {
        while (newPos < 0)
            newPos += dimension;
        while (newPos >= dimension)
            newPos -= dimension;
    }
    else
    {
        if (newPos < 0)
            newPos = 0;
        else if (newPos >= dimension)
            newPos = dimension - 1;
    }
    
    return newPos;
}

uint16_t RGBMatrixRenderer::newPositionX(uint16_t x, uint16_t increment, bool wrap)
{
    return newPosition(x, increment, gridWidth, wrap);
}

uint16_t RGBMatrixRenderer::newPositionY(uint16_t y, uint16_t increment, bool wrap)
{
    return newPosition(y, increment, gridHeight, wrap);
}

uint8_t RGBMatrixRenderer::blendColour(uint8_t start, uint8_t end, uint8_t step, uint8_t steps)
{
  
  uint8_t blend = start + (end - start) * step / steps;

  return blend;

}
