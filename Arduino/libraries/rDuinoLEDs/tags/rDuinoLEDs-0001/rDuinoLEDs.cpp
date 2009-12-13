/* Library for LEDs on the rDuino LEDHead */

#include <avr/io.h>
#include "rDuinoLEDs.h"

rDuinoLEDs::rDuinoLEDs()
{
  PORTC &= 0b00000011;  // all LED off
  PORTB &= 0b11111100;
  DDRC  |= 0b11111100;  // set as outputs
  DDRB  |= 0b00000011;
}


void rDuinoLEDs::set(uint8_t LEDnum, uint8_t value)
{
  if (LEDnum > 1)
    if (value)
      PORTC |= (1 << LEDnum);
    else
      PORTC &= ~(1 << LEDnum);
  else
    if (value)
      PORTB |= (1 << LEDnum);
    else
      PORTB &= ~(1 << LEDnum);
}

void rDuinoLEDs::set(uint8_t value)
{
  PORTC = (PORTC & 0b00000011) | (value & 0b11111100);
  PORTB = (PORTB & 0b11111100) | (value & 0b00000011);
}

void rDuinoLEDs::toggle(uint8_t LEDnum)
{
  if (LEDnum > 1)
    PORTC ^= (1 << LEDnum);
  else
    PORTB ^= (1 << LEDnum);
}

// Instantiate single instance

rDuinoLEDs LEDS;
