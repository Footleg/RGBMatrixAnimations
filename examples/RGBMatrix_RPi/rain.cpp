/**************************************************************************************************
 * This is an example to demonstrate using an animation class with the RGB matrix library
 * from https://github.com/hzeller/rpi-rgb-led-matrix
 *
 * Based on the public domain demo example file by Henner Zeller, and extended by
 * Paul Fretwell - aka 'Footleg' to use the animation classes written by Footleg with the RGBMatrix
 * library.
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

#include <unistd.h>
#include <chrono>
#include <signal.h>

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "pixel-mapper.h"
#include "graphics.h"

#include "gravityparticles.h" //This is the animation class used to generate output for the display

using namespace rgb_matrix;

uint64_t prevTime  = 0;      // Used for frames-per-second throttle

uint8_t backbuffer = 0;      // Index for double-buffered animation

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

// Get time stamp in microseconds.
uint64_t micros()
{
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
    return us; 
}

// RGB Matrix class which pass itself as a renderer implementation into the GOL class
// Passing as a reference into gol class, so need to dereference 'this' which is a pointer
// using the syntax *this
class Animation : public ThreadedCanvasManipulator, public RGBMatrixRenderer {
    public:
        Animation(Canvas *m, uint16_t width, uint16_t height, uint16_t delay_ms, int16_t accel_, uint16_t shake, uint16_t numParticles_, 
            int16_t velocity_, uint8_t bounce_)
            : ThreadedCanvasManipulator(m), RGBMatrixRenderer{width,height}, delay_ms_(delay_ms), animation(*this,shake,bounce_)
        {
            accel = accel_;
            velocity = velocity_;
            counter = 0;
            cycles = 100000;
            numParticles = numParticles_;
            if (numParticles_ > width) {
                removeNum = width;
            }
            else {
                removeNum = numParticles_-1;
            }
            
        }
        
        virtual ~Animation(){}

        void Run() {
            uint8_t MAX_FPS=1000/delay_ms_;    // Maximum redraw rate, frames/second

            //Set fixed acceleration
            animation.setAcceleration(0, -accel);

            const uint16_t totalCols = gridWidth/1.4;
            uint16_t* cols;
            uint16_t* vels;
            uint8_t* lengths;
            // Allocate memory
            vels = new uint16_t[totalCols];
            cols = new uint16_t[totalCols];
            lengths = new uint8_t[totalCols];
            //Set initial column configurations
            for (uint16_t x=0; x<totalCols; ++x) {
                cols[x] = gridWidth; //Initialise to off grid
                vels[x] = random_int16(velocity/4,velocity);
                lengths[x] = 0;
            }

            //Create palette of smoothly blending colours
            uint16_t brightness = 255;
            uint8_t red = 0;
            uint8_t green = 255;
            uint8_t blue = 0;
            uint16_t colID = 0;
            uint8_t shadeSize = 8;
            for (uint16_t i = 0; i <= 255; i++){
                //Green to yellow
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    red = uint16_t(brightness * i / 255);
                    green = brightness;
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }
            for (uint16_t i = 0; i <= 255; i++){
                //Yellow to red
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    red = brightness;
                    green = uint16_t(brightness * (255-i) / 255);
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }
            for (uint16_t i = 0; i <= 255; i++){
                //Red to magenta
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    red = brightness;
                    blue = uint16_t(brightness * i / 255);
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }
            for (uint16_t i = 0; i <= 255; i++){
                //Magenta to blue
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    red = uint16_t(brightness * (255-i) / 255);
                    blue = brightness;
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }
            for (uint16_t i = 0; i <= 255; i++){
                //Blue to cyan
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    green = uint16_t(brightness * i / 255);
                    blue = brightness;
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }
            for (uint16_t i = 0; i <= 255; i++){
                //Cyan to green
                for (uint8_t j = 0; j < shadeSize; j++){
                    brightness = random_int16(50,255);
                    green = brightness;
                    blue = uint16_t(brightness * (255-i) / 255);
                    colID = getColourId(RGB_colour(red,green,blue));
                }
            }

            const uint16_t stepSize = 1;
            uint16_t totalColours = colID;
            colID = 1;
            while (running() && !interrupt_received) {
                //Add new particles to top row
                if (animation.getParticleCount() < numParticles) {
                    counter++;
                    if (counter >= stepSize) counter = 0;
                    //Add particle to each column
                    for (int i=0; i<totalCols; i++) {
                        //Check if column has expired
                        if (lengths[i] < 1) {
                            //Reset column
                            bool colClear = false;
                            uint16_t newPos = 0;
                            while (colClear == false){
                                colClear = true;
                                newPos = random_int16(0,gridWidth);
                                for (uint16_t x=0; x<totalCols; ++x) {
                                    if (newPos == cols[x]){
                                        colClear = false;
                                    }
                                }
                            }
                            cols[i] = newPos;
                            lengths[i] = random_int16(8,24);
                            vels[i] = random_int16(velocity/4,velocity);
                        }
                        //Check position is clear on top row
                        uint8_t tries = 0;
                        while (tries < 1) {
                            //Add particle if free position was found
                            if ( (getPixelValue((gridHeight-1) * gridWidth + cols[i])) == false ) {
                                //Cycle colour every time counter rolls over
                                if (counter == 0) colID++;
                                if (colID >= totalColours) colID = 1;
                                RGB_colour actualCol = getColour(colID);
                                animation.addParticle(cols[i], gridHeight-1, actualCol, 0, -vels[i] );
                                lengths[i]--;
                                tries = 255;
                                //fprintf(stderr, "New particle added. Brightness %d, ColourID %d, Actual %d,%d,%d.\n",brightness,colID,actualCol.r,actualCol.g,actualCol.b);
                            }
                            else {
                                tries++;
                            }
                        }
                        // if (tries < 255) {
                        //     char msg[50];
                        //     sprintf(msg, "Failed to find free position for new particle.\n");
                        //     outputMessage(msg);
                        // }
                        // else {
                        //     //Show new particle now as they only get updated if they move, so never appear if no forces
                        //     updateDisplay();
                        // }
                    }
// char msg[50];
// sprintf(msg, "Counter %d, Ratio %d.\n",counter,colourRatio);
// outputMessage(msg);
                }

                //Remove oldest particles if > max allowed
                
                if (animation.getParticleCount() > removeNum) {
                    //Remove any of the oldest particles which have reached the bottom
                    for (uint16_t i = 0; i<removeNum; i++) {
                        GravityParticles::Particle test = animation.getParticle(removeNum-1-i);
                        // char msg[50];
                        // sprintf(msg, "Testing particle at %d,%d.\n",test.x,test.y);
                        // outputMessage(msg);

                        if ( test.y == 0) {
                            animation.deleteParticle(removeNum-1-i);
                        }
                    }
                }
                
                animation.runCycle();
                // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
                // calculations are non-deterministic (don't always take the same amount
                // of time, depending on their current states), this helps ensure that
                // things like gravity appear constant in the simulation.
                uint32_t t;
                

                while((t = micros() - prevTime) < (100000L / MAX_FPS));
                //fprintf(stderr,"Cycle time: %d\n", t );
                prevTime = micros();

                //Reset cycles before acceleration is changed based on speed of update
                cycles = 8000000 / t;
                if (accel < 5) cycles = 2*cycles;
            }
        }

        virtual void showPixels() {
            //Nothing to do for RGB matrix type displays as pixel changes are shown immediately
        }

        virtual void outputMessage(char msg[]) {
            fprintf(stderr,msg);
        }
        
        virtual void msSleep(int delay_ms) {
            usleep(delay_ms * 1000);
        }

        virtual int16_t random_int16(int16_t a, int16_t b) {
            return a + rand()%(b-a);
        }

    private:
        int delay_ms_;
        GravityParticles animation;
        int16_t accel;
        int16_t velocity;
        uint16_t numParticles;
        uint32_t counter, cycles;
        uint8_t removeNum;

        virtual void setPixel(uint16_t x, uint16_t y, RGB_colour colour) 
        {
            canvas()->SetPixel(x, gridHeight - y - 1, colour.r, colour.g, colour.b);
        }
};


static int usage(const char *progname) {
    fprintf(stderr, "usage: %s <options> [optional parameter]\n",
            progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr,
            "\t-m <msecs>     : Milliseconds pause between updates.\n"
            "\t-t <seconds>   : Run for these number of seconds, then exit.\n"
            "\t-n <number>    : Number of random grains of sand in addition to square blocks.\n"
            "\t-v <number>    : Initial velocity of particles.\n"
            "\t-g <number>    : Gravity force (0-100 is sensible, but takes higher).\n"
            "\t-s <number>    : Random shake force (0-100 is sensible, but takes higher).\n"
            "\t-e <number>    : Bounce energy (0-255). Max 255 means no loss of energy.\n");

    rgb_matrix::PrintMatrixFlags(stderr);

    fprintf(stderr, "Example:\n\t%s -n 64 -g 10 -s 5 -t 10 \n"
            "Runs demo for 10 seconds\n", progname);
    return 1;
}


int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 10;
    int accel = 1;
    int shake = 0;
    int velocity = 6000;
    int numParticles = 4000;
    uint8_t bounce = 0;

    srand(time(NULL));
 
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;

    // These are the defaults when no command-line flags are given.
    matrix_options.rows = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;

    // First things first: extract the command line flags that contain
    // relevant matrix options.
    if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return usage(argv[0]);
    }

    int opt;
    while ((opt = getopt(argc, argv, "dD:t:r:P:e:v:g:s:c:n:p:b:m:LR:")) != -1) {
        switch (opt) {
        case 't':
        runtime_seconds = atoi(optarg);
        break;

        case 'm':
        scroll_ms = atoi(optarg);
        break;

        case 'n':
        numParticles = atoi(optarg);
        break;

        case 'v':
        velocity = atoi(optarg);
        break;

        case 'g':
        accel = atoi(optarg);
        break;

        case 's':
        shake = atoi(optarg);
        break;

        case 'e':
        bounce = atoi(optarg);
        break;

        // These used to be options we understood, but deprecated now. Accept
        // but don't mention in usage()
        case 'R':
        fprintf(stderr, "-R is deprecated. "
                "Use --led-pixel-mapper=\"Rotate:%s\" instead.\n", optarg);
        return 1;
        break;

        case 'L':
        fprintf(stderr, "-L is deprecated. Use\n\t--led-pixel-mapper=\"U-mapper\" --led-chain=4\ninstead.\n");
        return 1;
        break;

        case 'd':
        runtime_opt.daemon = 1;
        break;

        case 'r':
        fprintf(stderr, "Instead of deprecated -r, use --led-rows=%s instead.\n",
                optarg);
        matrix_options.rows = atoi(optarg);
        break;

        case 'P':
        matrix_options.parallel = atoi(optarg);
        break;

        case 'c':
        fprintf(stderr, "Instead of deprecated -c, use --led-chain=%s instead.\n",
                optarg);
        matrix_options.chain_length = atoi(optarg);
        break;

        case 'p':
        matrix_options.pwm_bits = atoi(optarg);
        break;

        case 'b':
        matrix_options.brightness = atoi(optarg);
        break;

        default: /* '?' */
        return usage(argv[0]);
        }
    }

    RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
        return 1;

    printf("Size: %dx%d. Hardware gpio mapping: %s\n",
            matrix->width(), matrix->height(), matrix_options.hardware_mapping);

    Canvas *canvas = matrix;

    // The ThreadedCanvasManipulator objects are filling
    // the matrix continuously.
    ThreadedCanvasManipulator *image_gen = NULL;
    image_gen = new Animation(canvas, canvas->width(), canvas->height(), scroll_ms, accel, shake, numParticles, velocity, bounce);

    // Set up an interrupt handler to be able to stop animations while they go
    // on. Note, each demo tests for while (running() && !interrupt_received) {},
    // so they exit as soon as they get a signal.
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    // Image generating demo is crated. Now start the thread.
    image_gen->Start();

    // Now, the image generation runs in the background. We can do arbitrary
    // things here in parallel. In this demo, we're essentially just
    // waiting for one of the conditions to exit.
    if (runtime_seconds > 0) {
        sleep(runtime_seconds);
    } else {
        // The
        printf("Press <CTRL-C> to exit and reset LEDs\n");
        while (!interrupt_received) {
        sleep(1); // Time doesn't really matter. The syscall will be interrupted.
        }
    }

    // Stop image generating thread. The delete triggers
    delete image_gen;
    delete canvas;

    printf("\%s. Exiting.\n",
            interrupt_received ? "Received CTRL-C" : "Timeout reached");
    return 0;
}
