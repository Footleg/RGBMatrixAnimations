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

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "pixel-mapper.h"
#include "graphics.h"

#include "golife.h" //This is the animation class used to generate output for the display

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

// RGB Matrix class which pass itself as a renderer implementation into the GOL class
// Passing as a reference into gol class, so need to dereference 'this' which is a pointer
// using the syntax *this
class Animation : public ThreadedCanvasManipulator, public RGBMatrixRenderer {
    public:
        Animation(Canvas *m, uint16_t width, uint16_t height, uint16_t delay_ms, uint8_t fade_steps, uint8_t start_pattern_, uint8_t patternSpacingX_, uint8_t patternSpacingY_)
            : ThreadedCanvasManipulator(m), RGBMatrixRenderer{width,height}, delay_ms_(delay_ms), animation(*this,fade_steps,delay_ms,start_pattern_,patternSpacingX_,patternSpacingY_)
        {}

        virtual ~Animation(){}

        void Run() {
            while (running() && !interrupt_received) {
                animation.runCycle();
                usleep(delay_ms_ * 1000); // ms
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
        GameOfLife animation;

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
            "\t-f <steps>                : Number of steps in colour fades (1=no fades).\n"
            "\t-s <start-pattern>        : Preset starting pattern (0=random).\n"
            "\t-h <number>               : Number of copies of pattern vertically (1..n).\n"
            "\t-w <number>               : Number of copies of pattern across width (1..n).\n"
            );

    rgb_matrix::PrintMatrixFlags(stderr);

    fprintf(stderr, "Example (with fades disabled):\n\t%s -t 10 -f 1 \n"
            "Runs demo for 10 seconds\n", progname);
    return 1;
}


int main(int argc, char *argv[]) {
    int runtime_seconds = -1;
    int scroll_ms = 30;
    uint8_t fade_steps = 50;
    uint8_t start_pattern = 0;
    uint8_t patX = 1;
    uint8_t patY = 1;

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
    while ((opt = getopt(argc, argv, "dD:t:r:f:s:w:h:P:c:p:b:m:LR:")) != -1) {
        switch (opt) {
        case 't':
        runtime_seconds = atoi(optarg);
        break;

        case 'm':
        scroll_ms = atoi(optarg);
        break;

        case 'f':
        fade_steps = atoi(optarg);
        break;

        case 's':
        start_pattern = atoi(optarg);
        break;

        case 'w':
        patX = atoi(optarg);
        break;

        case 'h':
        patY = atoi(optarg);
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
    image_gen = new Animation(canvas, canvas->width(), canvas->height(), scroll_ms, fade_steps, start_pattern, patX, patY);

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
