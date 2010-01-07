/**********************************************************/
/* Serial Bootloader for Atmel megaAVR Controllers        */
/* With optional load from SD card through Rogue SD       */
/* module.                                                */
/*                                                        */
/* RogueSDBoot.c                                          */
/*                                                        */
/*                                                        */
/* 20090308: integrated Mega changes into main bootloader */
/*           source by D. Mellis                          */
/* 20080930: hacked for Arduino Mega (with the 1280       */
/*           processor, backwards compatible)             */
/*           by D. Cuartielles                            */
/* 20070626: hacked for Arduino Diecimila (which auto-    */
/*           resets when a USB connection is made to it)  */
/*           by D. Mellis                                 */
/* 20060802: hacked for Arduino by D. Cuartielles         */
/*           based on a previous hack by D. Mellis        */
/*           and D. Cuartielles                           */
/*                                                        */
/* ------------------------------------------------------ */
/*                                                        */
/* based on stk500boot.c                                  */
/* Copyright (c) 2003, Jason P. Kyle                      */
/* All rights reserved.                                   */
/*                                                        */
/* This program is free software; you can redistribute it */
/* and/or modify it under the terms of the GNU General    */
/* Public License as published by the Free Software       */
/* Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                    */
/*                                                        */
/* This program is distributed in the hope that it will   */
/* be useful, but WITHOUT ANY WARRANTY; without even the  */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A   */
/* PARTICULAR PURPOSE.  See the GNU General Public        */
/* License for more details.                              */
/*                                                        */
/* You should have received a copy of the GNU General     */
/* Public License along with this program; if not, write  */
/* to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */
/*                                                        */
/* Licence can be viewed at                               */
/* http://www.fsf.org/licenses/gpl.txt                    */
/*                                                        */
/**********************************************************/


// INCLUDES
#include <inttypes.h>
#include <ctype.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include "usart.h"
#include "roguesd.h"
//#include "rDuino-hw.h"
#include "Duemilanove-328p-hw.h"

#define SDBOOT 1
//#define DEBUG 1

// BUG FIXES:

// avr-libc uses signature from '644 (libc ver 09/03/13)
#if defined(__AVR_ATmega644P__)
#warning "FIXME: avr-libc uses signature from '644 (libc ver 09/03/13)"
#undef SIGNATURE_2
#define SIGNATURE_2 0x0A
#endif

// MACROS
#define PSTRU(s) (__extension__({static unsigned char __c[] PROGMEM = (s); &__c[0];}))
#if defined(RAMPZ)
#define P_read_byte(a) pgm_read_byte_far(a)
#else
#define P_read_byte(a) pgm_read_byte(a)
#endif

// CONSTANTS
#define MAX_ERROR_COUNT 5
#define NUM_LED_FLASHES 2
#define MAX_TIME_COUNT F_CPU>>4


// UART baud rate
#ifndef BAUD_RATE
#define BAUD_RATE    57600
#endif


/* SW_MAJOR and MINOR needs to be updated from time to time to avoid warning message from AVR Studio */
/* never allow AVR Studio to do an update !!!! */
#define HW_VER   0x02
#define SW_MAJOR 0x01
#define SW_MINOR 0x10



/* onboard LED is used to indicate, that the bootloader was entered (3x flashing) */
/* if monitor functions are included, LED goes on after monitor was entered */
#ifndef LED_DDR
#define LED_DDR           DDRB
#define LED_PORT          PORTB
#define LED_PIN           PINB
#define LED               PINB0
#endif

#define LEDInitialize()   LED_DDR |= (1 << LED)
#define LEDOn()           LED_PORT |= (1 << LED)
#define LEDOff()          LED_PORT &= ~(1 << LED)


#define PAGE_SIZE         (SPM_PAGESIZE/2)

// FIXME: REMOVE unions (non-portable)

/* some variables */
union address_union {
  uint16_t word;
  uint8_t  byte[2];
} address;

union length_union {
  uint16_t word;
  uint8_t  byte[2];
} length;

struct flags_struct {
  unsigned eeprom : 1;
  unsigned rampz  : 1;
} flags;

uint8_t linebuff[48];
uint8_t buff[256];
uint8_t address_high;

uint8_t pagesz=0x80;

uint8_t i;
uint8_t bootuart = 0;

uint8_t error_count = 0;

void (*app_start)(void) = 0x0000;


#ifdef DEBUG
// DEBUG
void debugout(unsigned char data)
{
	while(!(UCSR1A & (1 << UDRE1)));
	UDR1 = data;
}

void debughex(char ch)
{
  char ah;

  ah = ch >> 4;

  if (ah >= 0x0a)
    ah = ah - 0x0a + 'a';
  else
    ah += '0';
  
  ch &= 0x0f;
  if (ch >= 0x0a)
    ch = ch - 0x0a + 'a';
  else
    ch += '0';
  
  debugout(ah);
  debugout(ch);
}

void debugstr_P(const prog_uchar *s)
{
  register uint8_t c;

  while ((c = P_read_byte(s)))
  {
    debugout(c);
    s++;
  }
}

#else

#define debugout(x)
#define debughex(x)
#define debugstr_P(x)

#endif



#define putch(c) usartSendByte(c)
/*
void putch(char ch)
{
  while (!(UCSR0A & _BV(UDRE0)));
  UDR0 = ch;
}
*/

/*
char getch(void)
{
  uint32_t count = 0;

	LED_PORT &= ~(1 << LED);          // toggle LED to show activity

  while (!(UCSR0A & _BV(RXC0)))
  {
    count++;

    if (count > MAX_TIME_COUNT)
      app_start();
  }

  LED_PORT |= (1 << LED);          // toggle LED to show activity

  return UDR0;
}
*/


uint8_t getch(void)
{
  uint32_t count = 0;

  LEDOff();

  while (!usartHasData())
  {
    count++;

    if (count > MAX_TIME_COUNT)
      app_start();
  }

  LEDOn();

  return usartGetByte();
}


void putstr_P(const prog_uchar *str)
{
  register uint8_t c;

  while ((c = P_read_byte(str)))
  {
    putch(c);
    str++;
  }
}


char gethexnib(void)
{
  char a;

  a = getch();
  putch(a);

  if (a >= 'a')
    return (a - 'a' + 0x0a);
  else if (a >= '0')
    return (a - '0');

  return a;
}


char gethex(void)
{
  return (gethexnib() << 4) + gethexnib();
}


void puthex(char ch)
{
  char ah;

  ah = ch >> 4;

  if (ah >= 0x0a)
    ah = ah - 0x0a + 'a';
  else
    ah += '0';
  
  ch &= 0x0f;
  if (ch >= 0x0a)
    ch = ch - 0x0a + 'a';
  else
    ch += '0';
  
  putch(ah);
  putch(ch);
}


void getNch(uint8_t count)
{
  uint8_t i;

  for (i=0; i<count; i++)
  {
//    while (!(UCSR0A & _BV(RXC0)));
//    UDR0;
    usartGetByte();
  }
}


void byte_response(uint8_t val)
{
  if (getch() == ' ')
  {
    putch(0x14);
    putch(val);
    putch(0x10);
  }
  else
  {
    if (++error_count == MAX_ERROR_COUNT)
      app_start();
  }
}


void nothing_response(void)
{
  if (getch() == ' ')
  {
    putch(0x14);
    putch(0x10);
  }
  else
  {
    if (++error_count == MAX_ERROR_COUNT)
      app_start();
  }
}

void flash_led(uint8_t count)
{
  while (count--)
  {
    LEDOn();
    _delay_ms(100);
    LEDOff();
    _delay_ms(100);
  }
}


void boot_program_page(uint32_t page, uint8_t *buf)
{
  uint16_t i;
  uint8_t sreg;

  // Disable interrupts.

  sreg = SREG;
  cli();

  eeprom_busy_wait();

  boot_page_erase(page);
  boot_spm_busy_wait();       // Wait until the memory is erased.

  for (i=0; i<SPM_PAGESIZE; i+=2)
  {
      // Set up little-endian word.

      uint16_t w = *buf++;
      w += (*buf++) << 8;
  
      boot_page_fill(page + i, w);
  }

  boot_page_write(page);      // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.

  boot_rww_enable();

  // Re-enable interrupts (if they were ever enabled).

  SREG = sreg;
}


#ifdef SDBOOT
uint8_t asciiToBinary(uint8_t *s)
{
  uint8_t val = s[0];

  if (isdigit(s[0]))
    val = val - '0';
  else
    val = val - 'A' + 10;

  val <<= 4;

  val += s[1];

  if (isdigit(s[1]))
    val = val - '0';
  else
    val = val - 'A' + 10;

  return val;
}


int8_t intelHexParse(void)
{
	// Assumptions:
	// 1. No random records - i.e. all records are contiguous, and there is no address jumping back or forth
	// 2. Data starts at a page boundary - i.e. the first record starts at the beginning of a page
	// 3. Records are not longer than 16 bytes
	// 4. First record starts at address 0.

  unsigned char memory[SPM_PAGESIZE]; // create temporary page
	unsigned int mem_count = 0;
  unsigned char overflow[16];
  unsigned char overflow_count = 0;
  unsigned char linebuff[48];
  unsigned char *p;
  unsigned char n;
  unsigned int page, nextAddress;
  unsigned int count, type, i;
  register uint8_t checksum = 0;

  type = 0;
  page = 0;

  nextAddress = 0;

  do
  {
    if (rogueSDReadln(linebuff) > 0)
    {
      checksum = 0;
      p = linebuff;

      if (*p++ == ':')
      {
        // good to go
        // now get record length
        checksum = count = asciiToBinary(p);
        p += 2;

        // now address
        checksum += (n = asciiToBinary(p));
        nextAddress = n << 8;
        p += 2;
        checksum += (n = asciiToBinary(p));
        nextAddress += n;
        p += 2;

        // now record type
        checksum += (type = asciiToBinary(p));
        p += 2;

				if (type == 0)
				{
  				// ascertain our page
  				page = nextAddress/SPM_PAGESIZE;

					// data record, so add the bytes to the buffer
        	// read "count" bytes into buffer
        	for (i = 0; i < count; i++)
        	{
          	checksum += (n = asciiToBinary(p));
          	if (mem_count < SPM_PAGESIZE)
          		memory[mem_count++] = n;
          	else  // we had a short record somewhere before, now we have to keep an overflow buffer
          		overflow[overflow_count++] = n;
          	p += 2;
        	}
	        // check against crc
	        checksum += asciiToBinary(p);

	        if (checksum)
	        {
	          return -1;
	        }
        }

        if (mem_count == SPM_PAGESIZE)
        {

debugstr_P(PSTRU("Programming page: "));
debughex(page);
debugout('\r');

        	// Program the page
        	boot_program_page(page*SPM_PAGESIZE, memory);
        	
        	mem_count = 0;

					// copy over any overflow bytes
        	if (overflow_count > 0)
        	{
        		for (i = 0; i < overflow_count; i++)
        			memory[i] = overflow[i];
        		
        		mem_count = overflow_count;
        		overflow_count = 0;
        	}
        }
      }
      else
      {
        // record didn't begin with ':'
        return -1;
      }
    }
    else
    {
      // bad read
      return -1;
    }
  } while(type != 1);

	// now check if we have any left over data to be programmed

	if (mem_count > 0)
	{
    i = mem_count;
    while (i < SPM_PAGESIZE)
    {
      memory[i++] = 0xFF;
    }

debugstr_P(PSTRU("Remainder page: "));
debughex(page);
debugout('\r');

  	boot_program_page(page*SPM_PAGESIZE, memory);
	}

  return 0;
}
#endif


/* Bootloader start */
int main(void) __attribute__ ((OS_main));  // Stop stack push
int main(void)
{
  uint8_t ch,ch2;
  uint16_t w;


  // Disable watchdog timer, for runaway sketches that enable the WDT.
  MCUSR = 0;
  wdt_disable();

#ifdef SDBOOT
  // Pull up for boot-pin
  HW_SET_SDBOOT_PIN_PULLUP();
#endif


#ifdef DEBUG
  // set up second serial port
  UCSR1B = (1 << RXEN1) + (1 << TXEN1);
  UBRR1 = (F_CPU/9600/16L) - 1;
#endif


  usartInit();

  usartSetBitrate(BAUD_RATE);

  LEDInitialize();

  flash_led(NUM_LED_FLASHES);


// Load HEX file into program memory.
// Filename is "sketch.hex" in root directory of card.
#ifdef SDBOOT

  register uint8_t i = 250;

  while(i > 0)
  {
    if (HW_SDBOOT_PIN_IS_HIGH())
      break;  // bounced
    i--;
  }

  if (i == 0)
  {
    usartSetBitrate(9600);

    rogueSDSync();

    if (rogueSDOpen(PSTRU("/sketch.hex")) == 1)
    {
      intelHexParse();

      rogueSDCloseAll();

      // now just sit here until jumper is removed
      while (HW_SDBOOT_PIN_IS_LOW());
      // jump to app
      app_start();
    }
  }
#endif

  for (;;)
  {

    /* get character from UART */
    ch = getch();

    /* A bunch of if...else if... gives smaller code than switch...case ! */

    /* Hello is anyone home ? */ 
    if (ch=='0')
    {
      nothing_response();
    }


    /* Request programmer ID */
    else if (ch=='1')
    {
      if (getch() == ' ')
      {

//        putch(0x14);
        putstr_P(PSTRU("\024AVR ISP\020"));
//        putch(0x10);
      }
      else
      {
        if (++error_count == MAX_ERROR_COUNT)
          app_start();
      }
    }


    /* AVR ISP/STK500 board commands  DON'T CARE so default nothing_response */
    else if (ch=='@')
    {
      ch2 = getch();
      if (ch2>0x85) getch();
      nothing_response();
    }


    /* AVR ISP/STK500 board requests */
    else if (ch=='A')
    {
      ch2 = getch();
      if (ch2==0x80) byte_response(HW_VER);    // Hardware version
      else if (ch2==0x81) byte_response(SW_MAJOR); // Software major version
      else if (ch2==0x82) byte_response(SW_MINOR); // Software minor version
      else if (ch2==0x98) byte_response(0x03);   // Unknown but seems to be required by avr studio 3.56
      else byte_response(0x00);       // Covers various unnecessary responses we don't care about
    }


    /* Device Parameters  DON'T CARE, DEVICE IS FIXED  */
    else if (ch=='B')
    {
      getNch(20);
      nothing_response();
    }


    /* Parallel programming stuff  DON'T CARE  */
    else if (ch=='E')
    {
      getNch(5);
      nothing_response();
    }


    /* P: Enter programming mode  */
    /* R: Erase device, don't care as we will erase one page at a time anyway.  */
    else if (ch=='P' || ch=='R')
    {
      nothing_response();
    }


    /* Leave programming mode  */
    else if (ch=='Q')
    {
      nothing_response();
    }


    /* Set address, little endian. EEPROM in bytes, FLASH in words  */
    /* Perhaps extra address bytes may be added in future to support > 128kB FLASH.  */
    /* This might explain why little endian was used here, big endian used everywhere else.  */
    else if (ch=='U')
    {
      address.byte[0] = getch();
      address.byte[1] = getch();
      nothing_response();
    }


    /* Universal SPI programming command, disabled.  Would be used for fuses and lock bits.  */
    else if (ch=='V')
    {
      if (getch() == 0x30)
      {
        getch();
        ch = getch();
        getch();
        if (ch == 0)
        {
          byte_response(SIGNATURE_0);
        }
        else if (ch == 1)
        {
          byte_response(SIGNATURE_1); 
        }
        else
        {
          byte_response(SIGNATURE_2);
        } 
      }
      else
      {
        getNch(3);
        byte_response(0x00);
      }
    }


    /* Write memory, length is big endian and is in bytes  */
    else if (ch=='d')
    {
      length.byte[1] = getch();
      length.byte[0] = getch();
      flags.eeprom = 0;
      if (getch() == 'E') flags.eeprom = 1;
      for (w=0; w<length.word; w++)
      {
        buff[w] = getch();                          // Store data in buffer, can't keep up with serial data stream whilst programming pages
      }
      if (getch() == ' ')
      {
        if (flags.eeprom)
        {                                           //Write to EEPROM one byte at a time
          address.word <<= 1;
          for(w=0; w<length.word; w++)
          {
            eeprom_write_byte((void *)address.word,buff[w]);
            address.word++;
          }     
        }
        else
        {                                           //Write to FLASH one page at a time
          boot_program_page((uint32_t)(address.word)<<1, buff);
        }
        putch(0x14);
        putch(0x10);
      }
      else
      {
        if (++error_count == MAX_ERROR_COUNT)
          app_start();
      }   
    }


    /* Read memory block mode, length is big endian.  */
    else if (ch=='t')
    {
      length.byte[1] = getch();
      length.byte[0] = getch();
  #if defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__)
      if (address.word>0x7FFF) flags.rampz = 1;   // No go with m256, FIXME
      else flags.rampz = 0;
  #endif
      address.word = address.word << 1;         // address * 2 -> byte location
      if (getch() == 'E') flags.eeprom = 1;
      else flags.eeprom = 0;
      if (getch() == ' ')
      {                                         // Command terminator
        putch(0x14);
        for (w=0; w<length.word; w++)
        {                                       // Can handle odd and even lengths okay
          if (flags.eeprom)
          {                                     // Byte access EEPROM read
            putch(eeprom_read_byte((void *)address.word));
            address.word++;
          }
          else
          {
            if (!flags.rampz) putch(pgm_read_byte_near(address.word));
  #if defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__)
            else putch(pgm_read_byte_far(address.word + 0x10000));
            // Hmmmm, yuck  FIXME when m256 arrvies
  #endif
            address.word++;
          }
        }
        putch(0x10);
      }
    }


    /* Get device signature bytes  */
    else if (ch=='u')
    {
      if (getch() == ' ')
      {
        putch(0x14);
        putch(SIGNATURE_0);
        putch(SIGNATURE_1);
        putch(SIGNATURE_2);
        putch(0x10);
      }
      else
      {
        if (++error_count == MAX_ERROR_COUNT)
          app_start();
      }
    }


    /* Read oscillator calibration byte */
    else if (ch=='v')
    {
      byte_response(0x00);
    }

    else if (++error_count == MAX_ERROR_COUNT)
    {
      app_start();
    }
  } /* end of forever loop */
}

/* end of file RogueSDBoot.c */
