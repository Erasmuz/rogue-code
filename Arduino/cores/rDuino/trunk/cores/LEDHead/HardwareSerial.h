/*
  HardwareSerial.h - Hardware serial library for Arduino

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

*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>

#include "SerialBase.h"

struct _fifo;

class HardwareSerial : public SerialBase
{
  private:
    volatile _fifo *_rxfifo;
    volatile _fifo *_txfifo;
    volatile uint16_t *_ubrr;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_udr;
  public:
    HardwareSerial(
      volatile _fifo *rxfifo,
      uint8_t *rxbuff,
      volatile _fifo *txfifo,
      uint8_t *txbuff,
      volatile uint16_t *ubrr,
      volatile uint8_t *ucsra,
      volatile uint8_t *ucsrb,
      volatile uint8_t *udr);
    void begin(long);
    void end();
    uint8_t available(void);
    int read(void);
    int peek(void);
    void flush(void);
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;

#endif

