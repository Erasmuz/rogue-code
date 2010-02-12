/*
  HardwareSerial.cpp - Hardware serial library for Arduino

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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "HardwareSerial.h"

// Some constants for the UARTS
#define RXCIE RXCIE0
#define UDRIE UDRIE0

#define RXEN  RXEN0
#define TXEN  TXEN0

#define UDRE  UDRE0



// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer, in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 16

struct _fifo {
	uint8_t	idx_w;
	uint8_t	idx_r;
	uint8_t	count;
	uint8_t *buff;
};

uint8_t _txbuff[TX_BUFFER_SIZE];
uint8_t _txbuff1[TX_BUFFER_SIZE];
uint8_t _rxbuff[RX_BUFFER_SIZE];
uint8_t _rxbuff1[RX_BUFFER_SIZE];

volatile _fifo __txfifo;
volatile _fifo __txfifo1;
volatile _fifo __rxfifo;
volatile _fifo __rxfifo1;


void __fifo_nq(unsigned char c, volatile _fifo *fifo, uint8_t buffersize)
{
	uint8_t n, i;

	n = fifo->count;
	if (n < buffersize)
	{
		fifo->count = ++n;
		i = fifo->idx_w;
		fifo->buff[i++] = c;
		if (i >= buffersize)
			i = 0;
		fifo->idx_w = i;
	}
}

// MUST check that fifo.count > 0 before calling this
uint8_t __fifo_dq(volatile _fifo *fifo, uint8_t buffersize)
{
	uint8_t d = 0;
	uint8_t i;

  i = fifo->idx_r;
  d = fifo->buff[i++];
  fifo->count--;
  if (i >= buffersize)
	  i = 0;
  fifo->idx_r = i;

	return d;
}

ISR(SIG_USART_RECV)
{
	unsigned char c = UDR0;
  __fifo_nq(c, &__rxfifo, RX_BUFFER_SIZE);
}


ISR(SIG_USART1_RECV)
{
	unsigned char c = UDR1;
  __fifo_nq(c, &__rxfifo1, RX_BUFFER_SIZE);
}


/*
ISR(SIG_USART_DATA)
{
	uint8_t n, i;

	n = __txfifo.count;
	if (n > 0)
	{
		__txfifo.count = --n;
		i = __txfifo.idx_r;
		UDR0 = __txfifo.buff[i++];
		if (i >= TX_BUFFER_SIZE)
			i = 0;
		__txfifo.idx_r = i;
	}
	if (n == 0)
		UCSR0B &= (1 << UDRIE0);
}
*/

ISR(SIG_USART_DATA)
{
  if (__txfifo.count > 0)
    UDR0 = __fifo_dq(&__txfifo, TX_BUFFER_SIZE);

  if (__txfifo.count == 0)
    UCSR0B = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
}



ISR(SIG_USART1_DATA)
{
  if (__txfifo1.count > 0)
    UDR1 = __fifo_dq(&__txfifo1, TX_BUFFER_SIZE);

  if (__txfifo1.count == 0)
    UCSR1B = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
}


// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(
  volatile _fifo *rxfifo,
  uint8_t *rxbuff,
  volatile _fifo *txfifo,
  uint8_t *txbuff,
  volatile uint16_t *ubrr,
  volatile uint8_t *ucsra,
  volatile uint8_t *ucsrb,
  volatile uint8_t *udr)
{
  _rxfifo = rxfifo;
  _rxfifo->buff = rxbuff;
  _txfifo = txfifo;
  _txfifo->buff = txbuff;
  _ubrr = ubrr;
  _ucsra = ucsra;
  _ucsrb = ucsrb;
  _udr = udr;
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long speed)
{
	*_ubrr = ((F_CPU / 16 + speed / 2) / speed - 1);
  *_ucsrb = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
}

void HardwareSerial::end()
{
  *_ucsrb &= ~((1 << RXEN) | (1 << TXEN) | (1 << RXCIE));
}

uint8_t HardwareSerial::available(void)
{
  return _rxfifo->count;
}

int HardwareSerial::read(void)
{
  if (_rxfifo->count > 0)
    return (int) __fifo_dq(_rxfifo, RX_BUFFER_SIZE);
  else
    return -1;
}

int HardwareSerial::peek(void)
{
  if (_rxfifo->count > 0)
    return (int) _rxfifo->buff[_rxfifo->idx_r];
  else
    return -1;
}

void HardwareSerial::flush()
{
  cli();
  _rxfifo->idx_r = _rxfifo->idx_w = 0;
  sei();
}

void HardwareSerial::write(uint8_t c)
{
	uint8_t i;

	// We will block here until we have some space free in the FIFO
	while(_txfifo->count >= TX_BUFFER_SIZE);

  cli();
  __fifo_nq(c, _txfifo, TX_BUFFER_SIZE);
  *_ucsrb |= (1 << UDRIE);
  sei();
}


// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial Serial(&__rxfifo, _rxbuff, &__txfifo, _txbuff, &UBRR0, &UCSR0A, &UCSR0B, &UDR0);
HardwareSerial Serial1(&__rxfifo1, _rxbuff1, &__txfifo1, _txbuff1, &UBRR1, &UCSR1A, &UCSR1B, &UDR1);
