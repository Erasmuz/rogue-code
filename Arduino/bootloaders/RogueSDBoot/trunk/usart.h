//=============================================================================
// File:			usart.h
//=============================================================================


#ifndef USART_H
#define USART_H

// Initialize USART
extern void usartInit(void);

// Set USART bitrate
extern void usartSetBitrate(unsigned long bitrate);

// Check if USART has data
extern unsigned char usartHasData(void);

// Prepare USART for self-programming purposes
extern void usartWaitForByte(void);

// Receive one byte
extern unsigned char usartGetByte(void);

// Reply one byte as a response to the received frame
extern void usartSendByte(unsigned char data);

#endif // USART_H
