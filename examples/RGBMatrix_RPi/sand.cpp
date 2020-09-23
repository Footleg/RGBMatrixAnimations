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

#include "fallingsand.h" //This is the animation class used to generate output for the display

using namespace rgb_matrix;

uint32_t prevTime  = 0;      // Used for frames-per-second throttle

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
        Animation(Canvas *m, int width, int height, int delay_ms, int accel_, int shake, int numGrains)
            : ThreadedCanvasManipulator(m), RGBMatrixRenderer{width,height}, delay_ms_(delay_ms), animation(*this,shake,numGrains), 
              ax(0), ay(0)
        {
            
            accel = accel_;
            counter=1000;
            angle=-1;
            cycles = 100000;

            //Create some static pixels


            int id = 180;
            for (int x=0; x<10;x++) {
                animation.setStaticPixel(x+11,11,id);
                animation.setStaticPixel(x+11,20,id);
            }
            for (int y=0; y<10;y++) {
                animation.setStaticPixel(11,y+11,id);
                animation.setStaticPixel(20,y+11,id);
            }
            animation.setStaticPixel(15,11,0);
            animation.setStaticPixel(16,11,0);
            animation.setStaticPixel(15,20,0);
            animation.setStaticPixel(16,20,0);

            //Add grains
            for (int i=0; i<numGrains;i++) {
                animation.addGrain(1 + rand()%215);
            }
        }
        
        virtual ~Animation(){}

        void Run() {
            uint8_t MAX_FPS=1000/delay_ms_;    // Maximum redraw rate, frames/second

            while (running() && !interrupt_received) {
                //Update acceleration every 1000 cycles
                counter++;
                if (counter > cycles) {
                    counter = 0;
                    angle++;
                    fprintf(stderr,"Angle %d, Accel: %d,%d\n", angle, ax, ay );
                    switch(angle) {
                        case 0:
                            ax = accel;
                            ay = 0;
                            break;
                        case 1:
                            ax = 0;
                            ay = -accel;
                            break;
                        case 2:
                            ax = -accel;
                            ay = 0;
                            break;
                        case 3:
                            ax = 0;
                            ay = accel;
                            break;
                        case 4:
                            ax = accel;
                            ay = -accel;
                            break;
                        case 5:
                            ax = -accel;
                            ay = accel;
                            angle = -1;
                            break;
                    }
                    animation.setAcceleration(ax,ay);
                }


                
                animation.runCycle();
                // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
                // calculations are non-deterministic (don't always take the same amount
                // of time, depending on their current states), this helps ensure that
                // things like gravity appear constant in the simulation.
                uint32_t t;
                

                while((t = micros() - prevTime) < (100000L / MAX_FPS));
//                fprintf(stderr,"Cycle time: %d\n", t );
                prevTime = micros();
                cycles = 2500000 / t;
            }
        }

        virtual void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) 
        {
            canvas()->SetPixel(x, gridHeight - y - 1, r, g, b);
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

        virtual uint16_t random_uint(uint16_t a, uint16_t b) {
            return a + rand()%(b-a);
        }

    private:
        int delay_ms_;
        FallingSand animation;
        int16_t ax,ay, accel, angle;
        uint32_t counter, cycles;
};


static int usage(const char *progname) {
    fprintf(stderr, "usage: %s <options> [optional parameter]\n",
            progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr,
            "\t-m <msecs>     : Milliseconds pause between updates.\n"
            "\t-t <seconds>   : Run for these number of seconds, then exit.\n"
            "\t-n <number>    : Number of grains of sand.\n"
            "\t-g <number>    : Gravity force (0-100 is sensible, but takes higher).\n"
            "\t-s <number>    : Random shake force (0-100 is sensible, but takes higher).\n");

    rgb_matrix::PrintMatrixFlags(stderr);

    fprintf(stderr, "Example:\n\t%s -n 64 -g 10 -s 5 -t 10 \n"
            "Runs demo for 10 seconds\n", progname);
    return 1;
}


int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 10;
    int accel = 0;
    int shake = 0;
    int numGrains = 4;

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
    while ((opt = getopt(argc, argv, "dD:t:r:P:g:s:c:n:p:b:m:LR:")) != -1) {
        switch (opt) {
        case 't':
        runtime_seconds = atoi(optarg);
        break;

        case 'm':
        scroll_ms = atoi(optarg);
        break;

        case 'n':
        numGrains = atoi(optarg);
        break;

        case 'g':
        accel = atoi(optarg);
        break;

        case 's':
        shake = atoi(optarg);
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
    image_gen = new Animation(canvas, canvas->width(), canvas->height(), scroll_ms, accel, shake, numGrains);

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
