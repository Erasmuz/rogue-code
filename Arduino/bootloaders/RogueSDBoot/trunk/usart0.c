//=============================================================================
// File:			usart0.c
// Description:		USART0 interface.
//=============================================================================

#include <avr/io.h>
#include "usart.h"


//=============================================================================
// Initialize usart0 interface

void usartInit(void)
{
	// Enable Rx and Tx.
	UCSR0B = (1 << RXEN0) + (1 << TXEN0);
}


//=============================================================================
// Set the usart bitrate

void usartSetBitrate(unsigned long bitrate)
{
  UBRR0 = (F_CPU/bitrate/16L) - 1;
}


//=============================================================================
// check if byte is in usart data register

unsigned char usartHasData(void)
{
  return (UCSR0A & (1 << RXC0));
}


//=============================================================================
// wait for byte to arrive

void usartWaitForByte(void)
{
	while (!usartHasData());
}


//=============================================================================
// Get byte

unsigned char usartGetByte(void)
{
	usartWaitForByte();
	
	return UDR0;
}


//=============================================================================
// Send byte

void usartSendByte(unsigned char data)
{
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

