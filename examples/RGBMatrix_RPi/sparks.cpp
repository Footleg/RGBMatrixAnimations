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

// RGB Matrix class which passes itself as a renderer implementation into the Gravity Particles class
// Passing as a reference into the animation class, so need to dereference 'this' which is a pointer
// using the syntax *this
class Animation : public ThreadedCanvasManipulator, public RGBMatrixRenderer {
    public:
        Animation(Canvas *m, uint16_t width, uint16_t height, uint16_t delay_ms, int16_t accel_, uint16_t shake, uint16_t numParticles_, uint8_t bounce_)
            : ThreadedCanvasManipulator(m), RGBMatrixRenderer{width,height}, delay_ms_(delay_ms), animation(*this,shake,bounce_), 
              ax(0), ay(0)
        {
            accel = accel_;
            counter = 0;
            cycles = 100000;
            numParticles = numParticles_;
        }
        
        virtual ~Animation(){}

        void Run() {
            uint8_t MAX_FPS=1000/delay_ms_;    // Maximum redraw rate, frames/second

            RGB_colour yellow = {255,200,120};

            //Add grains in random positions with random initial velocities
            int randParticles = numParticles;
            int16_t maxVel = 10000;
            for (int i=0; i<randParticles; i++) {
                //animation.addParticle( getRandomColour() );
                int16_t vx = random_int16(-maxVel,maxVel+1);
                int16_t vy = random_int16(-maxVel,maxVel+1);
                if (vx > 0) {
                    vx += maxVel/5;  
                }
                else {
                    vx -= maxVel/5;
                }
                if (vy > 0) {
                    vy += maxVel/5;  
                }
                else {
                    vy -= maxVel/5;
                }
                animation.addParticle( yellow, vx, vy );
            }

            //Show LEDs and pause before starting animation
            updateDisplay();
            msSleep(100);

            //Set fixed accelreration
            ax = 0;
            ay = -accel;
            animation.setAcceleration(ax,ay);

            while (running() && !interrupt_received) {
                // //Update acceleration every few cycles
                // counter++;
                // if (counter > cycles) {
                //     counter = 0;
                // }
                
                animation.runCycle();
                // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
                // calculations are non-deterministic (don't always take the same amount
                // of time, depending on their current states), this helps ensure that
                // things like gravity appear constant in the simulation.
                uint32_t t;
                

                while((t = micros() - prevTime) < (100000L / MAX_FPS));
                //fprintf(stderr,"Cycle time: %d\n", t );
                prevTime = micros();

            }
        }

        void showPixels() {
            //Nothing to do for RGB matrix type displays as pixel changes are shown immediately
        }

        void outputMessage(char msg[]) {
            fprintf(stderr,msg);
        }
        
        void msSleep(int delay_ms) {
            usleep(delay_ms * 1000);
        }

        int16_t random_int16(int16_t a, int16_t b) {
            return a + rand()%(b-a);
        }

    private:
        int delay_ms_;
        GravityParticles animation;
        int16_t ax,ay, accel;
        uint16_t numParticles;
        uint32_t counter, cycles;

        void setPixel(uint16_t x, uint16_t y, RGB_colour colour) 
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
            "\t-n <number>    : Number of spark particles.\n"
            "\t-g <number>    : Gravity force (0-100 is sensible, but takes higher).\n"
            "\t-s <number>    : Random shake force (0-100 is sensible, but takes higher).\n"
            "\t-e <number>    : Bounce energy (0-255). Max 255 means no loss of energy.\n");

    rgb_matrix::PrintMatrixFlags(stderr);

    fprintf(stderr, "Example:\n\t%s -n 64 -g 10 -s 5 -e 200 -t 10 \n"
            "Runs demo for 10 seconds\n", progname);
    return 1;
}


int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 10;
    int accel = 1;
    int shake = 5;
    int numParticles = 40;
    uint8_t bounce = 250;

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
    while ((opt = getopt(argc, argv, "dD:t:r:P:e:g:s:c:n:p:b:m:LR:")) != -1) {
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
    image_gen = new Animation(canvas, canvas->width(), canvas->height(), scroll_ms, accel, shake, numParticles, bounce);

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
