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
{
    // Allocate memory for colour palette array
    palette = new RGB_colour[256];
    palette[0] = RGB_colour{0,0,0};
    coloursDefined = 0;
    
    // Allocate memory for pixels array
    img = new uint8_t[width * height];

    clearImage();

} //RGBMatrixRenderer

// default destructor
RGBMatrixRenderer::~RGBMatrixRenderer()
{
    delete [] palette;
    delete [] img;
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

RGB_colour RGBMatrixRenderer::getRandomColour()
{
    //Fetches a random colour from the palette if palette is full, otherwise returns a new one
    if (coloursDefined == 255) {
        return getColour(random_int16(0,255));
    }
    else {
        return newRandomColour();
    }

}

RGB_colour RGBMatrixRenderer::newRandomColour()
{
    // Init colour randomly
    RGB_colour colour;
    colour.r = random_int16(0,maxBrightness);
    colour.g = random_int16(0,maxBrightness);
    colour.b = random_int16(0,maxBrightness);
    uint8_t minBrightness = maxBrightness * 3 / 4;
    
    //Prevent colours being too dim
    if (colour.r<minBrightness && colour.g<minBrightness && colour.b<minBrightness) {
        uint8_t c = random_int16(0,3);
        switch (c) {
        case 0:
            colour.r = 200;
            break;
        case 1:
            colour.g = 200;
            break;
        case 2:
            colour.b = 200;
            break;
        }
    }
    
    //This method for string concatenation was used as it compiles on both g++ on Linux
    //and arduino platforms
    char msg[32];
    sprintf(msg, "New RGB colour  %d, %d, %d\n", colour.r,  colour.g, colour.b);
    outputMessage(msg);

    return colour;
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

RGB_colour RGBMatrixRenderer::blendColour(RGB_colour start, RGB_colour end, uint8_t step, uint8_t steps)
{
  
  uint8_t r = start.r + (end.r - start.r) * step / steps;
  uint8_t g = start.g + (end.g - start.g) * step / steps;
  uint8_t b = start.b + (end.b - start.b) * step / steps;

  return RGB_colour{r,g,b};

}

RGB_colour RGBMatrixRenderer::getColour(uint8_t id)
{
    RGB_colour colour = {0,0,0};

    if (id <= coloursDefined) {
        colour = palette[id];
    }

    return colour;
}

uint8_t RGBMatrixRenderer::getColourId(RGB_colour colour)
{
   //Search palette for matching colour (black is always zero)
    uint8_t id = 0;
    if ( (colour.r !=0) || (colour.g !=0) || (colour.b !=0) ) {
        for (uint16_t i=1; i<=coloursDefined; i++) {
/*
char msg[64];
sprintf(msg, "Searching palette %d (size %d))\n", i, coloursDefined);
outputMessage(msg);
*/
            if ( (palette[i].r == colour.r)
            && (palette[i].g == colour.g) 
            && (palette[i].b == colour.b) ) {
                id = i;
                break;
            }
        }
        
        //If match not found, add to palette if room
        if (id == 0) {
            if (coloursDefined < 255) {
                coloursDefined++;
/*                
char msg[64];
sprintf(msg, "Adding colour: %d, %d, %d (Total: %d)\n", colour.r,  colour.g, colour.b, coloursDefined);
outputMessage(msg);
*/
                palette[coloursDefined] = colour;
            }
            id = coloursDefined; //For now set to last colour even if we couldn't add another one
        }
    }
    /*
if (id > 0) {
char msg[64];
sprintf(msg, "Returned colour at index: %d\n", id);
outputMessage(msg);   
}
*/
    return id;
}


//Update Whole Matrix Display
void RGBMatrixRenderer::updateDisplay()
{
    // Update pixel data on display
//    uint16_t pixels = 0;
    for(int y=0; y<gridHeight; y++) {
        for(int x=0; x<gridWidth; x++) {
            uint8_t colcode = img[y*gridWidth + x];
            if (colcode) {
                setPixel(x,y,getColour(colcode));
//                pixels++;
            }
            else {
                setPixel(x,y,RGB_colour{0,0,0});
            }
        }
    }
    showPixels();
/*
    char msg[32];
    sprintf(msg, "Pixel count: %d\n", pixels);
    outputMessage(msg);
 */
}

void RGBMatrixRenderer::clearImage()
{
    //Clear img
    for (uint16_t i=0; i<gridWidth * gridHeight; i++) {
        img[i]=0;
    }
    //Wipe palette
    coloursDefined = 0;
}

uint8_t RGBMatrixRenderer::getPixelValue(uint16_t index)
{
    return img[index];
}

uint8_t RGBMatrixRenderer::getPixelValue(uint16_t x, uint16_t y)
{
    return img[y*gridWidth + x];
}

void RGBMatrixRenderer::setPixelValue(uint16_t index, uint8_t value)
{
    img[index] = value;
}

// Sets pixel colour in memory only (will not show changes until update display called)
void RGBMatrixRenderer::setPixelColour(uint16_t x, uint16_t y, RGB_colour colour)
{
    img[y * gridWidth + x] = getColourId(colour);
/*    char msg[80];
    sprintf(msg, "Set img pixel: %d Colour: %d,%d,%d\n", y * gridWidth + x, colour.r, colour.g, colour.b );
    outputMessage(msg);
 */
}

// Sets pixel colour directly on display. Faster and non-persistent as in memory display
// buffer does not get updated with the change
void RGBMatrixRenderer::setPixelInstant(uint16_t x, uint16_t y, RGB_colour colour)
{
    setPixel(x,y,colour);
}
