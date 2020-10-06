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
#include "fallingsand.h" //This is the animation class used to generate output for the display

// RGB Matrix class which passes itself as a renderer implementation into the animation class.
// Needs to be declared before the global variable accesses it in this file.
// Passed as a reference into the animation class, so need to dereference 'this' which is a pointer
// using the syntax *this
class Animation : public RGBMatrixRenderer {
    public:
        Animation(uint16_t width, uint16_t height, uint16_t shake)
            : RGBMatrixRenderer{width,height}, animSand(*this,shake)
        {
            //Set up falling sand grains and static pixels
            for (int x=0; x<5; x++){
                for (int y=0; y<5; y++){
                    animSand.addGrain(x,y+11,RGB_colour{255,255,0});
                    animSand.addGrain(x+11,y+11,RGB_colour{0,255,255});
                    animSand.addGrain(x+11,y,RGB_colour{255,0,0});
                    animSand.addGrain(x,y,RGB_colour{0,255,0});
                }
            }
            
        }

        virtual ~Animation(){}

        void setUnicorn(UnicornHD unicornHAT){
            unicorn = unicornHAT;
        }
        void animationStep() {
            
            animSand.runCycle();
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
        FallingSand animSand;

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

/***** Global Variables *****/
UnicornHD unicorn;
Animation animation(16,16,10);

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
    animation.updateDisplay();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    //Call animation class method to run a cycle
    animation.animationStep();
    //SHOULD USE FPS delay timing here
    delay(20);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////