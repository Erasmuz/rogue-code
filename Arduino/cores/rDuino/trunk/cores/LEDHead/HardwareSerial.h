/* $Id: HardwareSerial.h 106 2010-06-11 21:39:44Z bhagman@roguerobotics.com $

  Hardware Serial Library Class
  Redesigned for RX and TX buffering using FIFO (by Alexander Brevig)
  by Brett Hagman
  http://www.roguerobotics.com/
  bhagman@roguerobotics.com


  Original Source:
    HardwareSerial.h - Hardware serial library for Wiring - 2006 Nicholas Zambetti
  Modifications:
    23 November 2006 by David A. Mellis

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>
#include "wiring_private.h"
#include "Stream.h"
#include "FIFO.h"

#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 16

#if defined(__AVR_ATmega1280__)
#define SerialPorts 4
#define Serial_RX_vect USART0_RX_vect
#define Serial1_RX_vect USART1_RX_vect
#define Serial2_RX_vect USART2_RX_vect
#define Serial3_RX_vect USART3_RX_vect
#define Serial_TX_vect USART0_UDRE_vect
#define Serial1_TX_vect USART1_UDRE_vect
#define Serial2_TX_vect USART2_UDRE_vect
#define Serial3_TX_vect USART3_UDRE_vect

#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
#define SerialPorts 2
#define Serial_RX_vect USART0_RX_vect
#define Serial1_RX_vect USART1_RX_vect
#define Serial_TX_vect USART0_UDRE_vect
#define Serial1_TX_vect USART1_UDRE_vect

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
#define SerialPorts 1
#define Serial_RX_vect USART_RX_vect
#define Serial_TX_vect USART_UDRE_vect

#elif defined(__AVR_ATmega8__)
#define SerialPorts 1
#define Serial_RX_vect USART_RXC_vect
#define Serial_TX_vect USART_UDRE_vect

#endif


ISR(Serial_RX_vect);
ISR(Serial_TX_vect);
#if SerialPorts > 1
ISR(Serial1_RX_vect);
ISR(Serial1_TX_vect);
#endif
#if SerialPorts > 2
ISR(Serial2_RX_vect);
ISR(Serial2_TX_vect);
#endif
#if SerialPorts > 3
ISR(Serial3_RX_vect);
ISR(Serial3_TX_vect);
#endif

class HardwareSerial : public Stream
{
  friend void Serial_RX_vect();
  friend void Serial_TX_vect();
#if SerialPorts > 1
  friend void Serial1_RX_vect();
  friend void Serial1_TX_vect();
#endif
#if SerialPorts > 2
  friend void Serial2_RX_vect();
  friend void Serial2_TX_vect();
#endif
#if SerialPorts > 3
  friend void Serial3_RX_vect();
  friend void Serial3_TX_vect();
#endif

  private:
    FIFO<uint8_t,RX_BUFFER_SIZE> rxfifo;
    FIFO<uint8_t,TX_BUFFER_SIZE> txfifo;
    volatile uint16_t *_ubrr;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_udr;
  public:
    HardwareSerial(
      volatile uint16_t *ubrr,
      volatile uint8_t *ucsra,
      volatile uint8_t *ucsrb,
      volatile uint8_t *udr);

    void begin(long);
    void end();
    int available(void);
    int read(void);
    int peek(void);
    void flush(void);
    void write(uint8_t);
};

extern HardwareSerial Serial;

#if SerialPorts > 1
extern HardwareSerial Serial1;
#endif
#if SerialPorts > 2
extern HardwareSerial Serial2;
#endif
#if SerialPorts > 3
extern HardwareSerial Serial3;
#endif


#endif
