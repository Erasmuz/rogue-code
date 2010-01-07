//=============================================================================
// File:			usart1.c
// Description:		USART1 interface.
//=============================================================================

#include <avr/io.h>
#include "usart.h"


//=============================================================================
// Initialize usart0 interface

void usartInit(void)
{
	// Enable Rx and Tx.
	UCSR1B = (1 << RXEN1) + (1 << TXEN1);
}


//=============================================================================
// Set the usart bitrate

void usartSetBitrate(unsigned long bitrate)
{
  UBRR1 = (F_CPU/bitrate/16L) - 1;
}


//=============================================================================
// check if byte is in usart data register

unsigned char usartHasData(void)
{
  return (UCSR1A & (1 << RXC1));
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
	
	return UDR1;
}


//=============================================================================
// Send byte

void usartSendByte(unsigned char data)
{
	while(!(UCSR1A & (1 << UDRE1)));
	UDR1 = data;
}

