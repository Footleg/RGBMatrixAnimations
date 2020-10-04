/* Class to interface for the Pimoroni Unicorn HAT HD
 * Cloned from code published under and MIT license
 * by Chris Parrot in the ToZero library for his Teensy
 * to Pi GPIO interface boards:
 * https://github.com/ZodiusInfuser/ToZeroAdapters
 */
#include "UnicornHD.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS/DESTRUCTOR
////////////////////////////////////////////////////////////////////////////////////////////////////
UnicornHD::UnicornHD(uint8_t brightness)
: _brightness(brightness)
{
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////
void UnicornHD::Begin()
{
  pinMode(GPIO_SPI_CE0, OUTPUT);
  digitalWrite(GPIO_SPI_CE0, HIGH);

  ToZero_SPI_Setup();
  GPIO_SPI.begin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CRGB (&UnicornHD::Pixels())[NUM_LEDS]
{
  return _pixels;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UnicornHD::SetBrightness(uint8_t brightness)
{
  _brightness = brightness;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UnicornHD::Show()
{
  digitalWrite(GPIO_SPI_CE0, LOW);
  GPIO_SPI.transfer(SPI_ADDRESS);
  for(uint32_t i = 0; i < NUM_LEDS; i++)
  {
    GPIO_SPI.transfer(((uint16_t)_pixels[i].r * _brightness) / 255);
    GPIO_SPI.transfer(((uint16_t)_pixels[i].g * _brightness) / 255);
    GPIO_SPI.transfer(((uint16_t)_pixels[i].b * _brightness) / 255);
  }
  digitalWrite(GPIO_SPI_CE0, HIGH);
  delay(SPI_DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
