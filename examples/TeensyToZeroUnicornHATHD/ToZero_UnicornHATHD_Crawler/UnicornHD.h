/* Class to interface for the Pimoroni Unicorn HAT HD
 * Cloned from code published under and MIT license
 * by Chris Parrot in the ToZero library for his Teensy
 * to Pi GPIO interface boards:
 * https://github.com/ZodiusInfuser/ToZeroAdapters
 */

#ifndef _UNICORN_HD_h
#define _UNICORN_HD_h

/***** Includes *****/
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#if defined(__IMXRT1062__)
  #include <ToZero_4x_SPI.h>
#else
  #include <ToZero_3x_SPI.h>
#endif
#include <FastLED.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////////////////////////
class UnicornHD
{
  //--------------------------------------------------
  // Constants
  //--------------------------------------------------
public:
  static const uint32_t NUM_LEDS = 256;
private:
  static const uint8_t DEFAULT_BRIGHTNESS = 64;
  static const uint8_t SPI_ADDRESS = 0x72;
  static const uint32_t SPI_DELAY = 1000 / 120;


  //--------------------------------------------------
  // Constructors/Destructor
  //--------------------------------------------------
public:
  UnicornHD(uint8_t brightness = DEFAULT_BRIGHTNESS);


  //--------------------------------------------------
  // Methods
  //--------------------------------------------------
public:
  void Begin();

  CRGB(&Pixels())[NUM_LEDS];

  void SetBrightness(uint8_t brightness);

  void Show();  


  //--------------------------------------------------
  // Variables
  //--------------------------------------------------
private:
  CRGB _pixels[NUM_LEDS];
  uint8_t _brightness;
};
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
