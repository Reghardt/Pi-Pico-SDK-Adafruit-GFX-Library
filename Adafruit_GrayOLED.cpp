#include "Adafruit_GrayOLED.h"
#include <Adafruit_GFX.h>

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#define grayoled_swap(a, b) \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for I2C-interfaced OLED displays.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_GrayOLED::Adafruit_GrayOLED(i2c_inst_t *i2cBus, uint8_t addr, uint16_t w, uint16_t h)
    : Adafruit_GFX(w, h), buffer(NULL)
{
  i2cDevice = new Adafruit_I2CDevice(i2cBus, addr);
}

/*!
    @brief  Destructor for Adafruit_GrayOLED object.
*/
Adafruit_GrayOLED::~Adafruit_GrayOLED(void)
{
  if (buffer)
  {
    free(buffer);
    buffer = NULL;
  }

  if (i2cDevice)
    delete i2cDevice;
}

// LOW-LEVEL UTILS ---------------------------------------------------------

/*!
    @brief Issue single command byte to OLED
    @param cmd The single byte command
*/
void Adafruit_GrayOLED::send_cmd(uint8_t cmd)
{
  uint8_t buf[2] = {0x80, cmd}; // 1000 0000
  i2cDevice->write(buf, 2, false);
}

// Issue list of commands to GrayOLED
/*!
    @brief Issue multiple bytes of commands OLED,
    @param buf Pointer to cmd array buffer
    @param num The number of bytes in the cmd array buffer
*/
void Adafruit_GrayOLED::send_cmd_list(const uint8_t *buf, uint8_t num)
{
  for (int i = 0; i < num; i++)
    send_cmd(buf[i]);
}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
            Note that subclasses must call this before other begin() init
    @param  addr
            I2C address of corresponding oled display.
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required. Default if unspecified is 0x3C.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple oled displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
bool Adafruit_GrayOLED::_init()
{

  // attempt to malloc the bitmap framebuffer
  if ((!buffer) &&
      !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
  {
    return false;
  }

  clearDisplay();

  // set max dirty window
  window_x1 = 0;
  window_y1 = 0;
  window_x2 = WIDTH - 1;
  window_y2 = HEIGHT - 1;

  return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Adafruit_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: MONOOLED_BLACK, MONOOLED_WHITE or
   MONOOLED_INVERT.
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_GrayOLED::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height()))
  {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch (getRotation())
    {
    case 1:
      grayoled_swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      grayoled_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }

    // adjust dirty window
    window_x1 = window_x1 < x ? window_x1 : x; // min(window_x1, x);
    window_y1 = window_y1 < y ? window_y1 : y; // min(window_y1, y);
    window_x2 = window_x2 > x ? window_x2 : x; // max(window_x2, x);
    window_y2 = window_y2 > y ? window_y2 : y; // max(window_y2, y);

    switch (color)
    {
    case MONOOLED_WHITE:
      buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
      break;
    case MONOOLED_BLACK:
      buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
      break;
    case MONOOLED_INVERSE:
      buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
      break;
    }
  }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_GrayOLED::clearDisplay()
{
  memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
  // set max dirty window
  window_x1 = 0;
  window_y1 = 0;
  window_x2 = WIDTH - 1;
  window_y2 = HEIGHT - 1;
}

/*!
    @brief  Return color of a single pixel in display buffer.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @return true if pixel is set (usually MONOOLED_WHITE, unless display invert
   mode is enabled), false if clear (MONOOLED_BLACK).
    @note   Reads from buffer contents; may not reflect current contents of
            screen if display() has not been called.
*/
bool Adafruit_GrayOLED::getPixel(int16_t x, int16_t y)
{
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height()))
  {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch (getRotation())
    {
    case 1:
      grayoled_swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      grayoled_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    return (buffer[x + (y / 8) * WIDTH] & (1 << (y & 7)));
  }
  return false; // Pixel out of bounds
}

/*!
    @brief  Get base address of display buffer for direct reading or writing.
    @return Pointer to an unsigned 8-bit array, column-major, columns padded
            to full byte boundary if needed.
*/
uint8_t *Adafruit_GrayOLED::getBuffer(void) { return buffer; }

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white). Handy for testing!
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing MONOOLED_BLACK (value 0) pixels will actually draw
   white, MONOOLED_WHITE (value 1) will draw black.
*/
void Adafruit_GrayOLED::invertDisplay(bool i)
{
  send_cmd(i ? GRAYOLED_INVERTDISPLAY : GRAYOLED_NORMALDISPLAY);
}

/*!
    @brief  Adjust the display contrast.
    @param  level The contrast level from 0 to 0x7F
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void Adafruit_GrayOLED::setContrast(uint8_t level)
{
  uint8_t cmd[] = {GRAYOLED_SETCONTRAST, level};
  send_cmd_list(cmd, 2);
}

void Adafruit_GrayOLED::send_buf(uint8_t buf[], int buflen)
{
  uint8_t *temp_buf = (uint8_t *)malloc(buflen + 1);

  temp_buf[0] = 0x40;
  memcpy(temp_buf + 1, buf, buflen);

  i2cDevice->write(temp_buf, buflen + 1, false);

  free(temp_buf);
}
