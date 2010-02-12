/* SerialBase.h

  SerialBase - Abstract Base Class

  An abstract class which can be used for just
  about any type of serialized data -
  synchronous, or asynchronous - and provides
  a base class which allows for easy polymorphism.
 
  Inherits Print base class to enable print and
  println functionality.

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
0001    B Hagman    09/06/21 Initial coding

*************************************************/

#ifndef _SerialBase_h
#define _SerialBase_h

#include <stdint.h>
#include <Print.h>

/*************************************************
* Class Definition
*************************************************/

class SerialBase : public Print
{
  public:
    // available: returns number of bytes available for reading
    // 0 if no bytes
    virtual uint8_t available(void) = 0;

    // peek: returns the byte waiting at the front of the queue
    // -1 if no bytes/error
    virtual int peek(void) = 0;

    // read: returns 1 byte if available
    // -1 if no bytes/error
    virtual int read(void) = 0;

    // write: sends a single byte through serial interface
    // NOTE: this is also needed for the Print class
    virtual void write(uint8_t) = 0;

    // flush: clears any bytes that may be in buffer
    virtual void flush(void) = 0;


//    int32_t getnumber(uint8_t base);
//    int16_t getstring(char *str, uint8_t size);
//    int16_t getstring(char *str, uint8_t maxsize, uint8_t terminator);
//    int16_t getstring(char *str, uint8_t maxsize, uint8_t terminator, bool consume_terminator);
};


#endif
