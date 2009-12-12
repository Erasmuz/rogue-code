/* Tone.cpp

  A Tone Generator Library

  Written by Brett Hagman

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Version Modified By Date     Comments
------- ----------- -------- --------
0001    B Hagman    09/08/02 Initial coding

*************************************************/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <wiring.h>
#include <pins_arduino.h>
#include "Tone.h"

#if defined(__AVR_ATmega8__)
#define TCCR2A TCCR2
#define TCCR2B TCCR2
#define COM2A1 COM21
#define COM2A0 COM20
#define OCR2A OCR2
#define TIMSK2 TIMSK
#define OCIE2A OCIE2
#define TIMER2_COMPA_vect TIMER2_COMP_vect
#endif

// timerx_toggle_count:
//  > 0 - duration specified
//  = 0 - stopped
//  < 0 - infinitely (until stop() method called, or new play() called)

#if !defined(__AVR_ATmega8__)
volatile long timer0_toggle_count;
volatile uint8_t *timer0_pin_port;
volatile uint8_t timer0_pin_mask;
#endif

volatile long timer1_toggle_count;
volatile uint8_t *timer1_pin_port;
volatile uint8_t timer1_pin_mask;
volatile long timer2_toggle_count;
volatile uint8_t *timer2_pin_port;
volatile uint8_t timer2_pin_mask;

#if defined(__AVR_ATmega1280__)
volatile long timer3_toggle_count;
volatile uint8_t *timer3_pin_port;
volatile uint8_t timer3_pin_mask;
volatile long timer4_toggle_count;
volatile uint8_t *timer4_pin_port;
volatile uint8_t timer4_pin_mask;
volatile long timer5_toggle_count;
volatile uint8_t *timer5_pin_port;
volatile uint8_t timer5_pin_mask;
#endif


#if defined(__AVR_ATmega1280__)

#define AVAILABLE_TONE_PINS 6

// Leave timers 1, and zero to last.
const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 2, 3, 4, 5, 1, 0 };

#elif defined(__AVR_ATmega8__)

#define AVAILABLE_TONE_PINS 2

const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 2, 1 };

#else

#define AVAILABLE_TONE_PINS 3

// Leave timer 0 to last.
const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 2, 1, 0 };

#endif



// Initialize our pin count

uint8_t Tone::_tone_pin_count = 0;


void Tone::begin(uint8_t tonePin)
{
  if(_tone_pin_count < AVAILABLE_TONE_PINS)
  {
    _pin = tonePin;
    _timer = pgm_read_byte(tone_pin_to_timer_PGM + _tone_pin_count);
    _tone_pin_count++;

    // Set timer specific stuff
    // All timers in CTC mode
    // 8 bit timers will require changing presaclar values,
    // whereas 16 bit timers are set to ck/1 (no prescalar)
    // (minimum frequency is 123Hz @ 16MHz, 62Hz @ 8MHz)
    switch(_timer)
    {
#if !defined(__AVR_ATmega8__)
      case 0:
        // 8 bit timer
        TCCR0A = 0;
        TCCR0B = 0;
        bitWrite(TCCR0A, WGM01, 1);
        bitWrite(TCCR0B, CS00, 1);
        timer0_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer0_pin_mask = digitalPinToBitMask(_pin);
        break;
#endif

      case 1:
        // 16 bit timer
        TCCR1A = 0;
        TCCR1B = 0;
        bitWrite(TCCR1B, WGM12, 1);
        bitWrite(TCCR1B, CS10, 1);
        timer1_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer1_pin_mask = digitalPinToBitMask(_pin);
        break;
      case 2:
        // 8 bit timer
        TCCR2A = 0;
        TCCR2B = 0;
        bitWrite(TCCR2A, WGM21, 1);
        bitWrite(TCCR2B, CS20, 1);
        timer2_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer2_pin_mask = digitalPinToBitMask(_pin);
        break;

#if defined(__AVR_ATmega1280__)
      case 3:
        // 16 bit timer
        TCCR3A = 0;
        TCCR3B = 0;
        bitWrite(TCCR3B, WGM32, 1);
        bitWrite(TCCR3B, CS30, 1);
        timer3_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer3_pin_mask = digitalPinToBitMask(_pin);
        break;
      case 4:
        // 16 bit timer
        TCCR4A = 0;
        TCCR4B = 0;
        bitWrite(TCCR4B, WGM42, 1);
        bitWrite(TCCR4B, CS40, 1);
        timer4_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer4_pin_mask = digitalPinToBitMask(_pin);
        break;
      case 5:
        // 16 bit timer
        TCCR5A = 0;
        TCCR5B = 0;
        bitWrite(TCCR5B, WGM52, 1);
        bitWrite(TCCR5B, CS50, 1);
        timer5_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer5_pin_mask = digitalPinToBitMask(_pin);
        break;
#endif
    }
  }
  else
  {
    // disabled
    _timer = -1;
  }
}



// frequency (in hertz) and duration (in milliseconds).

void Tone::play(int frequency, unsigned long duration)
{
  int prescalar = 1;
  long toggle_count = 0;

  if(_timer > 0)
  {
    // Set the pinMode as OUTPUT
    pinMode(_pin, OUTPUT);
    
    // Set the prescalar for 8 bit timers
    if(_timer == 2)
    {
      if(frequency < 123)
      {
        prescalar = 1024;
        TCCR2B = 0b111;     // CSx2, CSx1, CSx0
      }
      else if(frequency < 245)
      {
        prescalar = 256;
        TCCR2B = 0b110;     // CSx2, CSx1
      }
      else if(frequency < 490)
      {
        prescalar = 128;
        TCCR2B = 0b101;     // CSx2, CSx0
      }
      else if(frequency < 980)
      {
        prescalar = 64;
        TCCR2B = 0b100;     // CSx2
      }
      else if(frequency < 3920)
      {
        prescalar = 32;
        TCCR2B = 0b011;     // CSx1, CSx0
      }
      else
      {
        prescalar = 8;
        TCCR2B = 0b010;     // CSx1
      }
    }

#if !defined(__AVR_ATmega8__)
    else if(_timer == 0)
    {
      if(frequency < 123)
      {
        prescalar = 1024;
        TCCR0B = 0b101;     // CSx2, CSx0
      }
      else if(frequency < 490)
      {
        prescalar = 256;
        TCCR0B = 0b100;     // CSx2
      }
      else if(frequency < 3920)
      {
        prescalar = 64;
        TCCR0B = 0b011;     // CSx1, CSx0
      }
      else
      {
        prescalar = 8;
        TCCR0B = 0b010;     // CSx1
      }
    }
#endif

    // Calculate the toggle count
    if(duration > 0)
    {
      toggle_count = 2 * frequency * duration / 1000;
    }
    else
    {
      toggle_count = -1;
    }

    // Set the OCR for the given timer,
    // set the toggle count,
    // then turn on the interrupts
    switch(_timer)
    {

#if !defined(__AVR_ATmega8__)
      case 0:
        OCR0A = (F_CPU / frequency) / 2 / prescalar - 1;
        timer0_toggle_count = toggle_count;
        bitWrite(TIMSK0, OCIE0A, 1);
        break;
#endif

      case 1:
        OCR1A = (F_CPU / frequency) / 2 - 1;
        timer1_toggle_count = toggle_count;
        bitWrite(TIMSK1, OCIE1A, 1);
        break;
      case 2:
        OCR2A = (F_CPU / frequency) / 2 / prescalar - 1;
        timer2_toggle_count = toggle_count;
        bitWrite(TIMSK2, OCIE2A, 1);
        break;

#if defined(__AVR_ATmega1280__)
      case 3:
        OCR3A = (F_CPU / frequency) / 2 - 1;
        timer3_toggle_count = toggle_count;
        bitWrite(TIMSK3, OCIE3A, 1);
        break;
      case 4:
        OCR4A = (F_CPU / frequency) / 2 - 1;
        timer4_toggle_count = toggle_count;
        bitWrite(TIMSK4, OCIE4A, 1);
        break;
      case 5:
        OCR5A = (F_CPU / frequency) / 2 - 1;
        timer5_toggle_count = toggle_count;
        bitWrite(TIMSK5, OCIE5A, 1);
        break;
#endif

    }
  }
}


void Tone::stop()
{
  switch(_timer)
  {
    case 0:
      TIMSK0 = 0;
      break;
    case 1:
      TIMSK1 = 0;
      break;
    case 2:
      TIMSK2 = 0;
      break;

#if defined(__AVR_ATmega1280__)
    case 3:
      TIMSK3 = 0;
      break;
    case 4:
      TIMSK4 = 0;
      break;
    case 5:
      TIMSK5 = 0;
      break;
#endif
  }

  digitalWrite(_pin, 0);
}


bool Tone::isPlaying(void)
{
  bool returnvalue = false;
  
  switch(_timer)
  {
    case 0:
      returnvalue = (timer0_toggle_count > 0) || (TIMSK0 & (1 << OCIE0A));
      break;
    case 1:
      returnvalue = (timer1_toggle_count > 0) || (TIMSK1 & (1 << OCIE1A));
      break;
    case 2:
      returnvalue = (timer2_toggle_count > 0) || (TIMSK2 & (1 << OCIE2A));
      break;

#if defined(__AVR_ATmega1280__)
    case 3:
      returnvalue = (timer3_toggle_count > 0) || (TIMSK3 & (1 << OCIE3A));
      break;
    case 4:
      returnvalue = (timer4_toggle_count > 0) || (TIMSK4 & (1 << OCIE4A));
      break;
    case 5:
      returnvalue = (timer5_toggle_count > 0) || (TIMSK5 & (1 << OCIE5A));
      break;
#endif

  }
  return returnvalue;
}


#if !defined(__AVR_ATmega8__)
ISR(TIMER0_COMPA_vect)
{
  if(timer0_toggle_count != 0)
  {
    // toggle the pin
    if(*timer0_pin_port & timer0_pin_mask) // pin is already high
    {
      *timer0_pin_port &= ~(timer0_pin_mask);
    }
    else
    {
      *timer0_pin_port |= timer0_pin_mask;
    }

    if(timer0_toggle_count > 0)
      timer0_toggle_count--;
  }
  else
  {
    TIMSK0 = 0;   // disable the interrupt
    *timer0_pin_port &= ~(timer0_pin_mask);  // keep pin low after stop
  }
}
#endif


ISR(TIMER1_COMPA_vect)
{
  if(timer1_toggle_count != 0)
  {
    // toggle the pin
    if(*timer1_pin_port & timer1_pin_mask) // pin is already high
    {
      *timer1_pin_port &= ~(timer1_pin_mask);
    }
    else
    {
      *timer1_pin_port |= timer1_pin_mask;
    }

    if(timer1_toggle_count > 0)
      timer1_toggle_count--;
  }
  else
  {
    TIMSK1 = 0;   // disable the interrupt
    *timer1_pin_port &= ~(timer1_pin_mask);  // keep pin low after stop
  }
}


ISR(TIMER2_COMPA_vect)
{

  if(timer2_toggle_count != 0)
  {
    // toggle the pin
    if(*timer2_pin_port & timer2_pin_mask) // pin is already high
    {
      *timer2_pin_port &= ~(timer2_pin_mask);
    }
    else
    {
      *timer2_pin_port |= timer2_pin_mask;
    }

    if(timer2_toggle_count > 0)
      timer2_toggle_count--;
  }
  else
  {
    TIMSK2 = 0;   // disable the interrupt
    *timer2_pin_port &= ~(timer2_pin_mask);  // keep pin low after stop
  }
}



#if defined(__AVR_ATmega1280__)

ISR(TIMER3_COMPA_vect)
{
  if(timer3_toggle_count != 0)
  {
    // toggle the pin
    if(*timer3_pin_port & timer3_pin_mask) // pin is already high
    {
      *timer3_pin_port &= ~(timer3_pin_mask);
    }
    else
    {
      *timer3_pin_port |= timer3_pin_mask;
    }

    if(timer3_toggle_count > 0)
      timer3_toggle_count--;
  }
  else
  {
    TIMSK3 = 0;   // disable the interrupt
    *timer3_pin_port &= ~(timer3_pin_mask);  // keep pin low after stop
  }
}

ISR(TIMER4_COMPA_vect)
{
  if(timer4_toggle_count != 0)
  {
    // toggle the pin
    if(*timer4_pin_port & timer4_pin_mask) // pin is already high
    {
      *timer4_pin_port &= ~(timer4_pin_mask);
    }
    else
    {
      *timer4_pin_port |= timer4_pin_mask;
    }

    if(timer4_toggle_count > 0)
      timer4_toggle_count--;
  }
  else
  {
    TIMSK4 = 0;   // disable the interrupt
    *timer4_pin_port &= ~(timer4_pin_mask);  // keep pin low after stop
  }
}

ISR(TIMER5_COMPA_vect)
{
  if(timer5_toggle_count != 0)
  {
    // toggle the pin
    if(*timer5_pin_port & timer5_pin_mask) // pin is already high
    {
      *timer5_pin_port &= ~(timer5_pin_mask);
    }
    else
    {
      *timer5_pin_port |= timer5_pin_mask;
    }

    if(timer5_toggle_count > 0)
      timer5_toggle_count--;
  }
  else
  {
    TIMSK5 = 0;   // disable the interrupt
    *timer5_pin_port &= ~(timer5_pin_mask);  // keep pin low after stop
  }
}

#endif
