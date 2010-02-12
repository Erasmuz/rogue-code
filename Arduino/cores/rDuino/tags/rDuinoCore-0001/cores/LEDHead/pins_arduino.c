/*
  pins_arduino.c - pin definitions for the Arduino board
  Part of Arduino / Wiring Lite

  Copyright (c) 2005 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: pins_arduino.c 565 2009-03-25 10:50:00Z dmellis $
*/

#include <avr/io.h>
#include "wiring_private.h"
#include "pins_arduino.h"

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.


#define PA 0
#define PB 1
#define PC 2
#define PD 3


// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] =
{
  &DDRA,
  &DDRB,
  &DDRC,
  &DDRD,
};

const uint8_t PROGMEM port_to_output_PGM[] =
{
  &PORTA,
  &PORTB,
  &PORTC,
  &PORTD,
};

const uint8_t PROGMEM port_to_input_PGM[] =
{
  &PINA,
  &PINB,
  &PINC,
  &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
  PD, /* D0 */
  PD,
  PD,
  PD,
  PD,
  PD,
  PD,
  PD,
  PB, /* D8 */
  PB,
  PB,
  PB,
  PB,
  PB,
  PA, /* D14/A0*/
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PA,
  PB, /* D22 */
  PB,
  PC, /* D24 */
  PC,
  PC,
  PC,
  PC,
  PC,
  PC,
  PC  /* D31 */
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
  _BV(0), /* D0, port D */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(2), /* D8, port B */
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* D14, port A */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7),
  _BV(0), /* D22, port B */
  _BV(1),
  _BV(0), /* D24, port C */
  _BV(1),
  _BV(2),
  _BV(3),
  _BV(4),
  _BV(5),
  _BV(6),
  _BV(7)
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
  NOT_ON_TIMER, // D0 - PD0
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  TIMER1B,      // D4 - PD4
  TIMER1A,      // D5 - PD5
  TIMER2B,      // D6 - PD6
  TIMER2A,      // D7 - PD7
  NOT_ON_TIMER, // D8 - PB2
  TIMER0A,      // D9 - PB3
  TIMER0B,      // D10 - PB4
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, // D14/A0 - PA0
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER, // D22 - PB0
  NOT_ON_TIMER,
  NOT_ON_TIMER, // D24 - PC0
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER,
  NOT_ON_TIMER
};
