/**************************************************************************************************
 * This is an example to demonstrate using an animation class with the UnicornHAT HD using a 
 * Teensy ToZero board from Zodius Infuzer
 * from https://github.com/ZodiusInfuser/ToZeroAdapters
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
#define NO_TO_ZERO_WARNINGS

#include <FastLED.h>
#include <ToZero.h>

/***** Project Includes *****/
#include "UnicornHD.h"
#include "golife.h" //This is the animation class used to generate output for the display

// RGB Matrix class which passes itself as a renderer implementation into the animation class.
// Needs to be declared before the global variable accesses it in this file.
// Passed as a reference into the animation class, so need to dereference 'this' which is a pointer
// using the syntax *this
class Animation : public RGBMatrixRenderer {
    public:
        Animation(uint16_t width, uint16_t height, uint16_t delay_ms, uint8_t fade_steps)
            : RGBMatrixRenderer{width,height}, animGOL(*this,fade_steps,delay_ms)
        {}

        virtual ~Animation(){}

        void setUnicorn(UnicornHD unicornHAT){
            unicorn = unicornHAT;
        }
        void animationStep() {
            animGOL.runCycle();
        }

        virtual void showPixels() {
            unicorn.Show();
        }

        virtual void outputMessage(char msg[]) {
            Serial.print(msg);
        }
        
        virtual void msSleep(int delay_ms) {
            delay(delay_ms);
        }

        int16_t random_int16(int16_t a, int16_t b) {
            return random(a,b);
        }

    private:
        UnicornHD unicorn;
        GameOfLife animGOL;

        virtual void setPixel(uint16_t x, uint16_t y, RGB_colour colour) 
        {
            CRGB col;
            col.red = colour.r;
            col.green = colour.g;
            col.blue = colour.b;
            unicorn.Pixels()[y*getGridHeight()+x] = col;
        }
};

/***** Global Constants *****/
static const unsigned long UNICORN_BRIGHTNESS = 128;
static const uint16_t delayms = 40; //Controls speed of animation. 40ms pause ~25fps

/***** Global Variables *****/
UnicornHD unicorn;
Animation animation(16,16,delayms,20); //Arguments for fades
//Animation animation(16,16,1,1); //Turbo test mode (looking for termination cases)

////////////////////////////////////////////////////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    
    //Initialise random seed from a floating analogue input
    randomSeed(analogRead(0));

    //Set up unicorn HAT
    unicorn.Begin();
    unicorn.SetBrightness(UNICORN_BRIGHTNESS);

    //Pass reference to unicorn HAT to animation class
    animation.setUnicorn(unicorn);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    //Call animation class method to run a cycle
    animation.animationStep();
    //Pause to slow down animation to desired speed
    delay(delayms);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
