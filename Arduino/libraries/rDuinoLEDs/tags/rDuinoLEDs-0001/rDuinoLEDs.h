#ifndef rDuinoLEDs_h
#define rDuinoLEDs_h

#include <stdint.h>

class rDuinoLEDs
{
  public:
    rDuinoLEDs();
    void set(uint8_t LEDnum, uint8_t value);
    void set(uint8_t value);
    void toggle(uint8_t LEDnum);
};

extern rDuinoLEDs LEDS;

#endif
