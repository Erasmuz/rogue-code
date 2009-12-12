/* Tone.cpp

  A Tone Generator Library

  A library to generate tones.
  'mega8, '168, '328 - only Pin 11
  'mega1280 - only Pin 10 

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
#include <wiring.h>
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

volatile unsigned long Tone::timer2_interrupt_count;
volatile bool Tone::playing = false;

// Generate a tone on digital pin 11 (or pin 10) with the given
// frequency (in hertz) and duration (in milliseconds).

void Tone::play(int frequency, unsigned long duration)
{
  int prescalar;

  // Set up output pin
#if defined(__AVR_ATmega1280__)
  pinMode(10, OUTPUT);
#else
  pinMode(11, OUTPUT);
#endif

  // Clear Timer on Compare Match (CTC) Mode, TOP = OCR2A
#if !defined(__AVR_ATmega8__)
  bitWrite(TCCR2B, WGM22, 0);
#endif
  bitWrite(TCCR2A, WGM21, 1);
  bitWrite(TCCR2A, WGM20, 0);
  
  // Toggle OC2A on Compare Match
  bitWrite(TCCR2A, COM2A1, 0);
  bitWrite(TCCR2A, COM2A0, 1);
  
  // Prescale based on frequency
  if(frequency < 330)
  {
    prescalar = 1024;
    bitWrite(TCCR2B, CS22, 1);
    bitWrite(TCCR2B, CS21, 1);
    bitWrite(TCCR2B, CS20, 1);
  }
  else if(frequency < 880)
  {
    prescalar = 256;
    bitWrite(TCCR2B, CS22, 1);
    bitWrite(TCCR2B, CS21, 1);
    bitWrite(TCCR2B, CS20, 0);
  }
  else if(frequency < 1480)
  {
    prescalar = 128;
    bitWrite(TCCR2B, CS22, 1);
    bitWrite(TCCR2B, CS21, 0);
    bitWrite(TCCR2B, CS20, 1);
  }
  else
  {
    prescalar = 64;
    bitWrite(TCCR2B, CS22, 1);
    bitWrite(TCCR2B, CS21, 0);
    bitWrite(TCCR2B, CS20, 0);
  }
  playing = true;
  
  OCR2A = (F_CPU / frequency) / 2 / prescalar - 1;

  if (duration != 0)
  {
    timer2_interrupt_count = 2 * frequency * duration / 1000;
    // Enable output compare match interrupt
    bitWrite(TIMSK2, OCIE2A, 1);
  }
}

void Tone::stop()
{
  // Phase Correct PWM
#if !defined(__AVR_ATmega8__)
  bitWrite(TCCR2B, WGM22, 0);
#endif
  bitWrite(TCCR2A, WGM21, 0);
  bitWrite(TCCR2A, WGM20, 1);

  // Disconnect OC2A
  bitWrite(TCCR2A, COM2A1, 0);
  bitWrite(TCCR2A, COM2A0, 0);
  
  // Prescale of 64
  bitWrite(TCCR2B, CS22, 1);
  bitWrite(TCCR2B, CS21, 0);
  bitWrite(TCCR2B, CS20, 0);

  // Reset output pin
#if defined(__AVR_ATmega1280__)
  pinMode(10, INPUT);
#else
  pinMode(11, INPUT);
#endif
  playing = false;
}

ISR(TIMER2_COMPA_vect)
{
  if (Tone::timer2_interrupt_count == 0) {
    bitWrite(TIMSK2, OCIE2A, 0); // disable the interrupt
    Tone::stop();
  }
  
  Tone::timer2_interrupt_count--;
}
