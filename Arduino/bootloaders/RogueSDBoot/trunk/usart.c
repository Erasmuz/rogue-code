//=============================================================================
// File:			usart.c
// Description:		USART interface.
//=============================================================================

#include <avr/io.h>
#include "usart.h"


//=============================================================================
// Initialize usart0 interface

void usartInit(void)
{
	// Enable Rx and Tx.
	UCSRB = (1 << RXEN) + (1 << TXEN);
}


//=============================================================================
// Set the usart bitrate

void usartSetBitrate(unsigned long bitrate)
{
  UBRR = (F_CPU/bitrate/16L) - 1;
}


//=============================================================================
// check if byte is in usart data register

unsigned char usartHasData(void)
{
  return (UCSRA & (1 << RXC));
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
	
	return UDR;
}


//=============================================================================
// Send byte

void usartSendByte(unsigned char data)
{
	while(!(UCSRA & (1 << UDRE)));
	UDR = data;
}

