#pragma once

#include "Adafruit_GFX.h"
#include "Adafruit_I2CDevice.h"

#define GRAYOLED_SETCONTRAST 0x81   ///< Generic contrast for almost all OLEDs
#define GRAYOLED_NORMALDISPLAY 0xA6 ///< Generic non-invert for almost all OLEDs
#define GRAYOLED_INVERTDISPLAY 0xA7 ///< Generic invert for almost all OLEDs

#define MONOOLED_BLACK 0   ///< Default black 'color' for monochrome OLEDS
#define MONOOLED_WHITE 1   ///< Default white 'color' for monochrome OLEDS
#define MONOOLED_INVERSE 2 ///< Default inversion command for monochrome OLEDS

class Adafruit_GrayOLED : public Adafruit_GFX
{
public:
  Adafruit_GrayOLED(i2c_inst_t *i2cBus, uint8_t addr, uint16_t w, uint16_t h);
  ~Adafruit_GrayOLED();

  /**
   @brief The function that sub-classes define that writes out the buffer to the display over I2C
   **/
  virtual void display(void) = 0;
  void clearDisplay(void);
  void invertDisplay(bool i);
  void setContrast(uint8_t contrastlevel);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  bool getPixel(int16_t x, int16_t y);
  uint8_t *getBuffer(void);

  void send_cmd(uint8_t cmd);
  void send_cmd_list(const uint8_t *buf, uint8_t num);
  void send_buf(uint8_t buf[], int buflen);

protected:
  bool _init();

  Adafruit_I2CDevice *i2cDevice = NULL; ///< The I2C interface BusIO device
  uint8_t *buffer = NULL;               ///< Internal 1:1 framebuffer of display mem

  int16_t window_x1; ///< Dirty tracking window minimum x
  int16_t window_y1; ///< Dirty tracking window minimum y
  int16_t window_x2; ///< Dirty tracking window maximum x
  int16_t window_y2; ///< Dirty tracking window maximum y

  int dcPin; ///< The Arduino pin connected to D/C (for SPI)
  int csPin; ///< The Arduino pin connected to CS (for SPI)
};
