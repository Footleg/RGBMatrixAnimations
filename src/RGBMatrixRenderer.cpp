/**************************************************************************************************
 * RGB Matrix Renderer abstract class 
 * 
 * This abstract class provides a template for building renderer classes to control LED Matrix
 * displays. It provides methods to set the colour of any LED/pixel in a grid, and to keep the
 * position of points within the bounds of the grid (with or without wrapping over the edges).
 * A series of animation classes use the interface defined in this class to output to displays.
 * By writing a renderer class for any LED matrix hardware (or on screen display) this allows the
 * animations classes to be used with many different hardware display devices without requiring
 * code changes in the animation class.
 * 
 * This class also implements a wrapping mode for panels arranged as faces of a cube, including
 * tracking the x and y components of acceleration in the plane of each face of the cube for a
 * 3D acceleration vector. The enables animations to update the motion of pixels on any panel 
 * based on the cube face they appear on, and wrapping of pixels across panel edges in cube mode.
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

#include "RGBMatrixRenderer.h"
#include <stdexcept>

// default constructor
RGBMatrixRenderer::RGBMatrixRenderer(uint16_t width, uint16_t height, uint8_t brightnessLimit, bool inCubeMode)
    : gridWidth(width), gridHeight(height), maxBrightness(brightnessLimit), cubeMode(inCubeMode)
{
    //Set panel size if in cube mode
    if (inCubeMode) {
        //Width has to be 3/2 of height in cube mode
        if (width == height * 3 / 2) {
            panelSize = height / 2;
        }
        else {
            //Unsupported panel arrangement for cube mode
            throw std::invalid_argument( "Cube mode only supports arrangements of 3 x 2 panels." );
        }
    }
    
    // Allocate memory for colour palette array
    palette = new RGB_colour[maxColours];
    palette[0] = RGB_colour{0,0,0};
    coloursDefined = 0;
    
    // Allocate memory for pixels array
    img = new uint16_t[width * height];

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
    if (coloursDefined >= maxColours) {
        return getColour(random_int16(0,maxColours));
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
/*
    char msg[32];
    sprintf(msg, "New RGB colour  %d, %d, %d\n", colour.r,  colour.g, colour.b);
    outputMessage(msg);
*/
    return colour;
}

//Method for a normal rectangular flat grid, updates a grid coordinate while keeping on the matrix, with optional wrapping
uint16_t RGBMatrixRenderer::newPosition(uint16_t position, uint16_t increment, uint16_t dimension, bool wrap)
{
    int16_t newPos = position + increment;

    // char msg[64];
    // sprintf(msg, "NewPos: Value  %d; Increment: %d; Limit: %d\n", position,  increment, dimension);
    // outputMessage(msg);

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

uint8_t RGBMatrixRenderer::getPanel(MovingPixel pixel)
{
    uint8_t panel = 0;

    uint8_t row = pixel.y / panelSize;
    uint8_t col = pixel.x / panelSize;

    panel = (row * gridWidth / panelSize) + col;
    
    return panel;
};

//Method for a cube, updates a grid coordinates while keeping on the matrix, with optional wrapping
MovingPixel  RGBMatrixRenderer::updatePosition(MovingPixel pixel, bool wrap)
{
    MovingPixel newPixel(0,0,0,0);

    //Update the pixel fine position (fraction of a pixel position)
    uint16_t fineInc = abs(pixel.fineX + pixel.vx);
// char msg1[20];
// sprintf(msg1, "FineIncX= %d ", fineInc);
// outputMessage(msg1);
    if (fineInc >= SUBPIXEL_RES) {
        //Moved at least one pixel position, so update
        int16_t wholePix = fineInc / SUBPIXEL_RES;
        int16_t partPix = fineInc - (wholePix*SUBPIXEL_RES);
        if (pixel.vx < 0) {
            wholePix = -wholePix;
            partPix = -partPix;
        }
        newPixel.x = newPositionX(pixel.x,wholePix,wrap);
        newPixel.fineX = partPix;
// char msg1[96];
// sprintf(msg1, "WholePix=%d PartPix=%d OldFine: %d,%d NewFine: %d,%d\n", wholePix, partPix, pixel.fineX, pixel.fineY, newPixel.fineX, newPixel.fineY);
// outputMessage(msg1);
    }
    else if (pixel.vx < 0) {
        newPixel.x = pixel.x;
        newPixel.fineX = -fineInc;
    }
    else {
        newPixel.x = pixel.x;
        newPixel.fineX = fineInc;
    }

    fineInc = abs(pixel.fineY + pixel.vy);
// char msg2[20];
// sprintf(msg2, "FineIncY= %d ", fineInc);
// outputMessage(msg2);
    if (fineInc >= SUBPIXEL_RES) {
        //Moved at least one pixel position, so update
        int16_t wholePix = fineInc / SUBPIXEL_RES;
        int16_t partPix = fineInc - (wholePix*SUBPIXEL_RES);
        if (pixel.vy < 0) {
            wholePix = -wholePix;
            partPix = -partPix;
        }
        newPixel.y = newPositionY(pixel.y,wholePix,wrap);
        newPixel.fineY = partPix;
// char msg1[96];
// sprintf(msg1, "WholePix=%d PartPix=%d OldFine: %d,%d NewFine: %d,%d\n", wholePix, partPix, pixel.fineX, pixel.fineY, newPixel.fineX, newPixel.fineY);
// outputMessage(msg1);
    }
    else if (pixel.vy < 0) {
        newPixel.y = pixel.y;
        newPixel.fineY = -fineInc;
    }
    else {
        newPixel.y = pixel.y;
        newPixel.fineY = fineInc;
    }
    
    newPixel.vx = pixel.vx;
    newPixel.vy = pixel.vy;

    if (cubeMode)
    {
        //For cube, the panels are arranged in 2 rows of 3. The bottom row of 3 panels represent 
        //3 sides with x being across, and y being up. The top row represents the top, back and bottom 
        //sides of the cube. x and y directions vary across these 3 panels wrt to the underlying 3 x 2
        //matrix.

        //Determine which panel the pixel is on now
        uint8_t panelRow = pixel.y / panelSize;
        uint8_t panelCol = pixel.x / panelSize;
        uint16_t newX;
        uint16_t newY;
        int16_t newFineX;
        //int16_t newFineY;

        //Determine which panel the new pixel is on (before cube wrapping translation is applied)
        uint8_t panelRowNew = newPixel.y / panelSize;
        uint8_t panelColNew = newPixel.x / panelSize;

// char msg1[96];
// sprintf(msg1, "Cube Mode. Wrap=%d On Panel: %d,%d Moving to panel: %d,%d pos: %d,%d ", wrap, panelCol, panelRow, panelColNew, panelRowNew, pixel.x, pixel.y);
// outputMessage(msg1);

        //Update wrapping if changed panel
        if (wrap == false) {
            //If not wrapping, then don't allow move off panel
            newPixel = pixel;
        } 
        else if (panelRow == panelRowNew) {
            if (panelRow == 0) {
                // Bottom Row
                if ( panelCol == 2 && panelColNew == 0) {
                    //Wrapped off RH edge of panel3, so transpose onto panel 5
                    newY = gridHeight - 1 - newPixel.x;
                    newX = panelSize + newPixel.y;
                    newPixel.x = newX;
                    newPixel.y = newY;
                    //Transpose part pixel positions
                    newFineX = newPixel.fineY;
                    newPixel.fineY = -newPixel.fineX;
                    newPixel.fineX = newFineX;
                    //Transpose X and Y velocities
                    newPixel.vx = pixel.vy;
                    newPixel.vy = -pixel.vx;
                }
                else if ( panelCol == 0 && panelColNew == 2) {
                    //Wrapped off LH edge of panel1, so transpose onto panel 5
                    newY = gridWidth - 1 - newPixel.x + panelSize;
                    newX = panelSize + newPixel.y;
                    newPixel.x = newX;
                    newPixel.y = newY;
                    //Transpose part pixel positions
                    newFineX = newPixel.fineY;
                    newPixel.fineY = -newPixel.fineX;
                    newPixel.fineX = newFineX;
                    //Transpose X and Y velocities
                    newPixel.vx = pixel.vy;
                    newPixel.vy = -pixel.vx;
                }
                // else {
                //     //Remains on row 0, so just copy velocity to new pixel
                //     newPixel.vx = pixel.vx;
                //     newPixel.vy = pixel.vy;
                // }
            }
            else { 
                //Upper Row
                if ( panelCol == 2 && panelColNew == 0) {
                    //Wrapped off RH edge of panel6, so transpose onto panel 2
                    newY = panelSize - 1 - newPixel.x;
                    newX = newPixel.y;
                    newPixel.x = newX;
                    newPixel.y = newY;
                    //Transpose part pixel positions
                    newFineX = newPixel.fineY;
                    newPixel.fineY = -newPixel.fineX;
                    newPixel.fineX = newFineX;
                    //Transpose X and Y velocities
                    newPixel.vx = pixel.vy;
                    newPixel.vy = -pixel.vx;
                }
                else if ( panelCol == 0 && panelColNew == 2) {
                    //Wrapped off LH edge of panel4, so transpose onto panel 5
                    newY = gridWidth - 1 - newPixel.x;
                    newX = newPixel.y;
                    newPixel.x = newX;
                    newPixel.y = newY;
                    //Transpose part pixel positions
                    newFineX = newPixel.fineY;
                    newPixel.fineY = -newPixel.fineX;
                    newPixel.fineX = newFineX;
                    //Transpose X and Y velocities
                    newPixel.vx = pixel.vy;
                    newPixel.vy = -pixel.vx;
                }
                // else {
                //     //Remains on row 1, so just copy velocity to new pixel
                //     newPixel.vx = pixel.vx;
                //     newPixel.vy = pixel.vy;
                // }
            }
        }
        else {
            //Change of panel row
            if (panelCol == panelColNew) {
                //Moving onto panel above or below (not diagonally)
                uint8_t panelColNewCube = 0;
                uint8_t panelRowNewCube = 0;
                int16_t translate = 0;
                if ( panelCol == 0 ) {
                    if ( panelRow == 0 ) {
                        //On panel 1
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 1 to panel 6
                            //Shift 2 panels to the right. Y wraps as normal so no change
                            translate = 2 * panelSize;
                        } 
                        else {
                            //Moving down from panel 1 to 4 (flip X and Y)
                            panelColNewCube = 0;
                            panelRowNewCube = 1;
                        }
                    } 
                    else {
                        //On panel 4
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 4 to panel 3
                            //Shift 2 panels to the right. Y wraps as normal so no change
                            translate = 2 * panelSize;
                        } 
                        else {
                            //Moving down from panel 4 to 1 (flip X and Y)
                            panelColNewCube = 0;
                            panelRowNewCube = 0;
                        }
                    }
                }
                else if ( panelCol == 1 ) {
                    if ( panelRow == 0 ) {
                        //On panel 2
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 2 to panel 6 (swap X and Y)
                            panelColNewCube = 2;
                            panelRowNewCube = 1;
                            translate = 1;
                        } 
                        else {
                            //Moving down from panel 2 to panel 4 (swap X and Y)
                            panelColNewCube = 0;
                            panelRowNewCube = 1;
                            translate = 1;
                        }
                    } 
                    else {
                        //On panel 5
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 5 to panel 3 (swap X and Y)
                            panelColNewCube = 2;
                            panelRowNewCube = 0;
                            translate = 1;
                        } 
                        else {
                            //Moving down from panel 5 to panel 1 (swap X and Y)
                            panelColNewCube = 0;
                            panelRowNewCube = 0;
                            translate = 1;
                        }
                    }
                }
                else {
                    if ( panelRow == 0 ) {
                        //On panel 3
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 3 to 6 (flip X and Y)
                            panelColNewCube = 2;
                            panelRowNewCube = 1;
                        } 
                        else {
                            //Moving down from panel 3 to panel 4
                            //Shift 2 panels to the left.
                            translate = - 2 * panelSize; 
                        }
                    } 
                    else {
                        //On panel 6
                        if ( pixel.vy > 0 ) {
                            //Moving up from panel 6 to panel 3 (flip X and Y)
                            panelColNewCube = 2;
                            panelRowNewCube = 0;
                        } 
                        else {
                            //Moving down from panel 6 to panel 1
                            //Shift 2 panels to the left.
                            translate = - 2 * panelSize; 
                        }
                    }
                }

                //Process operation
                if (translate == 0) {
                    //Flip X and Y within panel, then translate to position of final panel
                    newPixel.x = panelSize - 1 - (newPixel.x - panelColNew * panelSize) + panelColNewCube * panelSize;
                    newPixel.y = panelSize - 1 - (newPixel.y - panelRowNew * panelSize) + panelRowNewCube * panelSize;
                    //Reverse part pixel positions
                    newPixel.fineX = -newPixel.fineX;
                    newPixel.fineY = -newPixel.fineY;
                    //Reverse X and Y velocities
                    newPixel.vx = -pixel.vx;
                    newPixel.vy = -pixel.vy;
                }
                else if (translate == 1) {
                    //Swap X and Y within panel, flip Y, then translate to position of final panel
                    newX = panelSize - 1 - (newPixel.y - panelRowNew * panelSize) + panelColNewCube * panelSize;
                    newPixel.y = (newPixel.x - panelColNew * panelSize) + panelRowNewCube * panelSize;
                    newPixel.x = newX;
                    //Transpose part pixel positions
                    newFineX = -newPixel.fineY;
                    newPixel.fineY = newPixel.fineX;
                    newPixel.fineX = newFineX;
                    //Transpose X and Y velocities
                    newPixel.vx = -pixel.vy;
                    newPixel.vy = pixel.vx;
                }
                else {
                    //Translate x to new panel, Y wraps as for non-cube arrangement of flat panels, so no change in Y
                    newPixel.x = newPixel.x + translate; 
                    //Retain same X and Y velocities
                    newPixel.vx = pixel.vx;
                    newPixel.vy = pixel.vy;

                }
            }
            else {
                //Diagonal cases
                if ( panelCol == 0 ) {
                    if ( panelRow == 0 ) {
                        //On panel 1
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    } 
                    else {
                        //On panel 4
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    }
                }
                else if ( panelCol == 1 ) {
                    if ( panelRow == 0 ) {
                        //On panel 2
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    } 
                    else {
                        //On panel 5
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    }
                }
                else {
                    if ( panelRow == 0 ) {
                        //On panel 3
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    } 
                    else {
                        //On panel 6
                        if ( pixel.vy > 0 ) {
                            //Moving up
                        } 
                        else {
                            //Moving down
                        }
                    }
                }
            }
        }

    }
    
    return newPixel;
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

RGB_colour RGBMatrixRenderer::getColour(uint16_t id)
{
    RGB_colour colour = {0,0,0};

    if (id <= coloursDefined) {
        colour = palette[id];
    }

    return colour;
}

uint16_t RGBMatrixRenderer::getColourId(RGB_colour colour)
{
    uint16_t id = 0;
    uint16_t lowestScore = 100;
    uint16_t closestMatch = 0;

    //Search palette for matching colour (black is always zero)
    if ( (colour.r !=0) || (colour.g !=0) || (colour.b !=0) ) {
        for (uint16_t i=1; i<=coloursDefined; i++) {
/*
char msg1[64];
sprintf(msg1, "Searching palette %d (size %d))\n", i, coloursDefined);
outputMessage(msg1);
*/
            if ( (palette[i].r == colour.r)
            && (palette[i].g == colour.g) 
            && (palette[i].b == colour.b) ) {
                id = i;
                lowestScore = 0;
                closestMatch = id;
                break;
            }
            else if (coloursDefined < maxColours-1) {
                //Determine if closest match so far (only needed if palette is already full)
                uint16_t score = abs(palette[i].r - colour.r) + abs(palette[i].g - colour.g) + abs(palette[i].b - colour.b);
                if (score < lowestScore) {
                    lowestScore = score;
                    closestMatch = i;
                }
            }
        }
        
        //If match not found, add to palette if room
        if (id == 0) {
            if (coloursDefined < maxColours-1) {
                coloursDefined++;
/*                
char msg2[64];
sprintf(msg2, "Adding colour: %d, %d, %d (Total: %d)\n", colour.r,  colour.g, colour.b, coloursDefined);
outputMessage(msg2);
*/
                palette[coloursDefined] = colour;
                id = coloursDefined;
            }
            else {
                //Set to closest matching colour
                id = closestMatch;
char msg3[64];
sprintf(msg3, "Asked for (%d,%d,%d) but got (%d,%d,%d)\n", colour.r,  colour.g, colour.b, palette[id].r, palette[id].g, palette[id].b);
outputMessage(msg3);
            }
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
            uint16_t colcode = img[y*gridWidth + x];
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

uint16_t RGBMatrixRenderer::getPixelValue(uint16_t index)
{
    return img[index];
}

uint16_t RGBMatrixRenderer::getPixelValue(uint16_t x, uint16_t y)
{
    return img[y*gridWidth + x];
}

void RGBMatrixRenderer::setPixelValue(uint16_t index, uint16_t value)
{
    img[index] = value;
}

// Sets pixel colour in memory only (will not show changes until update display called)
void RGBMatrixRenderer::setPixelColour(uint16_t x, uint16_t y, RGB_colour colour)
{
    img[y * gridWidth + x] = getColourId(colour);
/*
    char msg[80];
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
