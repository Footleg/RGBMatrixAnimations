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
#include <signal.h>
#include <chrono>

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "pixel-mapper.h"
#include "graphics.h"

#include "gravitySimulation.h" //This is the animation class used to generate output for the display

using namespace rgb_matrix;

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
        Animation(Canvas *m, uint16_t width, uint16_t height, uint16_t delay_ms, uint16_t numBalls_, uint8_t maxRadius_, float force_)
            : ThreadedCanvasManipulator(m), RGBMatrixRenderer{width,height}, delay_ms_(delay_ms), animation(*this,maxRadius_)
        {
            numBalls = numBalls_;
            animation.forcePower = force_;
        }

        virtual ~Animation(){}

        void Run() {
            uint8_t MAX_FPS=1000/delay_ms_;    // Maximum redraw rate, frames/second
            uint64_t prevTime  = 0;            // Used for frames-per-second throttle

            animation.setMode(1);
            
            for (uint16_t i=0; i<numBalls; i++) {
                //animation.addParticle( getRandomColour() );
                animation.addBall();
            }

            while (running() && !interrupt_received) {
                animation.runCycle();
                // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
                // calculations are non-deterministic (don't always take the same amount
                // of time, depending on their current states), this helps ensure that
                // things like gravity appear constant in the simulation.
                uint32_t t;
                while((t = micros() - prevTime) < (1000000L / MAX_FPS)){
                    msSleep(1);
                };
                fprintf(stderr,"Max fps: %d; Cycle time: %d; Actual fps: %0.3f\n", MAX_FPS, t, (float)1000000L / t);
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
        uint16_t delay_ms_;
        GravitySimulation animation;
        uint16_t numBalls;

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
            "\t-m <msecs>                : Milliseconds pause between updates.\n"
            "\t-t <seconds>              : Run for these number of seconds, then exit.\n"
            "\t-n <number>               : Number of balls.\n"
            "\t-f <float>                : Attractive force power (set negative for repelling force).\n"
            "\t-s <number>               : Maximum radius of balls.\n");

    rgb_matrix::PrintMatrixFlags(stderr);

    fprintf(stderr, "Example (with fades disabled):\n\t%s -t 30 -n 3 -f 16 \n"
            "Runs demo for 30 seconds with 3 balls attracted to each other\n", progname);
    return 1;
}


int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 30;
    uint16_t numBalls = 2;
    uint8_t max_rad = 20;
    float force = 12;

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
    while ((opt = getopt(argc, argv, "dD:t:r:n:s:f:P:c:p:b:m:LR:")) != -1) {
        switch (opt) {
        case 't':
        runtime_seconds = atoi(optarg);
        break;

        case 'm':
        scroll_ms = atoi(optarg);
        break;

        case 'n':
        numBalls = atoi(optarg);
        break;

        case 's':
        max_rad = atoi(optarg);
        break;

        case 'f':
        force = atoi(optarg);
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
    image_gen = new Animation(canvas, canvas->width(), canvas->height(), scroll_ms, numBalls, max_rad, force);

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
