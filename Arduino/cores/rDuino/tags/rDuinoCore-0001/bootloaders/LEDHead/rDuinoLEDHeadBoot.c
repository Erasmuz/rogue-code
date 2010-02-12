/**********************************************************/
/* Serial Bootloader for Atmel megaAVR Controllers        */
/*                                                        */
/* ATmegaBOOT.c                                           */
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

/* $Id$ */

/* some includes */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#define MAX_ERROR_COUNT 5

/* set the UART baud rate */
#ifndef BAUD_RATE
#define BAUD_RATE   19200
#endif

/* SW_MAJOR and MINOR needs to be updated from time to time to avoid warning message from AVR Studio */
/* never allow AVR Studio to do an update !!!! */
#define HW_VER   0x02
#define SW_MAJOR 0x01
#define SW_MINOR 0x10

/* onboard LED is used to indicate, that the bootloader was entered (3x flashing) */
/* if monitor functions are included, LED goes on after monitor was entered */
#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_PIN  PINB
#define LED      PINB0

/* define various device id's */
/* manufacturer byte is always the same */
#define SIG1  0x1E  // Yep, Atmel is the only manufacturer of AVR micros.  Single source :(
//#if defined(__AVR_ATmega644P__)
#define SIG2  0x96
#define SIG3  0x0A

#define PAGE_SIZE   0x080U   //128 words
#define PAGE_SIZE_BYTES 0x100U   //256 bytes

/* function prototypes */
void putch(char);
char getch(void);
void getNch(uint8_t);
void byte_response(uint8_t);
void nothing_response(void);
char gethex(void);
void puthex(char);
void flash_led(uint8_t);

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

uint8_t buff[256];
uint8_t address_high;

uint8_t pagesz=0x80;

uint8_t i;
uint8_t bootuart = 0;

uint8_t error_count = 0;

void (*app_start)(void) = 0x0000;


/* main program starts here */
int main(void) __attribute__ ((OS_main));
int main(void)
{
  uint8_t ch,ch2;
  uint16_t w;

  asm volatile("nop\n\t");

#if defined __AVR_ATmega1280__
  /* the mega1280 chip has four serial ports ... we could eventually use any of them, or not? */
  /* however, we don't wanna confuse people, to avoid making a mess, we will stick to RXD0, TXD0 */
  bootuart = 1;
#endif

  //initialize our serial port.
  UBRR0L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1);
  UBRR0H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);

  /* Enable internal pull-up resistor on pin D0 (RX), in order
  to supress line noise that prevents the bootloader from
  timing out (DAM: 20070509) */
  DDRD &= ~(1 << PIND0);
  PORTD |= (1 << PIND0);

  /* set LED pin as output */
  LED_DDR |= (1 << LED);

  /* flash onboard LED to signal entering of bootloader */
#if defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__)
  // 4x for UART0, 5x for UART1
  flash_led(NUM_LED_FLASHES + bootuart);
#else
  flash_led(NUM_LED_FLASHES);
#endif

  /* forever loop */
  for (;;)
  {

    /* get character from UART */
    ch = getch();

    /* A bunch of if...else if... gives smaller code than switch...case ! */

    /* Hello is anyone home ? */ 
    if(ch=='0')
    {
      nothing_response();
    }


    /* Request programmer ID */
    /* Not using PROGMEM string due to boot block in m128 being beyond 64kB boundry  */
    /* Would need to selectively manipulate RAMPZ, and it's only 9 characters anyway so who cares.  */
    else if (ch=='1')
    {
      if (getch() == ' ')
      {
        putch(0x14);
        putch('A');
        putch('V');
        putch('R');
        putch(' ');
        putch('I');
        putch('S');
        putch('P');
        putch(0x10);
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
          byte_response(SIG1);
        }
        else if (ch == 1)
        {
          byte_response(SIG2); 
        }
        else
        {
          byte_response(SIG3);
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
          if (address.byte[1]>127) address_high = 0x01; //Only possible with m128, m256 will need 3rd address byte. FIXME
          else address_high = 0x00;
  #if defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__)
          RAMPZ = address_high;
  #endif
          address.word = address.word << 1;         //address * 2 -> byte location
          if ((length.byte[0] & 0x01)) length.word++; //Even up an odd number of bytes
          cli();          //Disable interrupts, just to be sure
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__)
          while(bit_is_set(EECR,EEPE));     //Wait for previous EEPROM writes to complete
  #else
          while(bit_is_set(EECR,EEWE));     //Wait for previous EEPROM writes to complete
  #endif
          asm volatile(
             "clr r17               \n\t" //page_word_count
             "lds r30,address       \n\t" //Address of FLASH location (in bytes)
             "lds r31,address+1     \n\t"
             "ldi r28,lo8(buff)     \n\t" //Start of buffer array in RAM
             "ldi r29,hi8(buff)     \n\t"
             "lds r24,length        \n\t" //Length of data to be written (in bytes)
             "lds r25,length+1      \n\t"
             "length_loop:          \n\t" //Main loop, repeat for number of words in block                             
             "cpi r17,0x00          \n\t" //If page_word_count=0 then erase page
             "brne  no_page_erase   \n\t"            
             "wait_spm1:            \n\t"
             "lds r16,%0            \n\t" //Wait for previous spm to complete
             "andi  r16,1           \n\t"
             "cpi r16,1             \n\t"
             "breq  wait_spm1       \n\t"
             "ldi r16,0x03          \n\t" //Erase page pointed to by Z
             "sts %0,r16            \n\t"
             "spm                   \n\t"              

             "wait_spm2:            \n\t"
             "lds r16,%0            \n\t" //Wait for previous spm to complete
             "andi  r16,1           \n\t"
             "cpi r16,1             \n\t"
             "breq  wait_spm2       \n\t"                  
  
             "ldi r16,0x11          \n\t" //Re-enable RWW section
             "sts %0,r16            \n\t"                  
             "spm                   \n\t"

             "no_page_erase:        \n\t"              
             "ld  r0,Y+             \n\t" //Write 2 bytes into page buffer
             "ld  r1,Y+             \n\t"              
                   
             "wait_spm3:            \n\t"
             "lds r16,%0            \n\t" //Wait for previous spm to complete
             "andi  r16,1           \n\t"
             "cpi r16,1             \n\t"
             "breq  wait_spm3       \n\t"
             "ldi r16,0x01          \n\t" //Load r0,r1 into FLASH page buffer
             "sts %0,r16            \n\t"
             "spm                   \n\t"
                   
             "inc r17               \n\t" //page_word_count++
             "cpi r17,%1            \n\t"
             "brlo  same_page       \n\t" //Still same page in FLASH
             "write_page:           \n\t"
             "clr r17               \n\t" //New page, write current one first
             "wait_spm4:            \n\t"
             "lds r16,%0            \n\t" //Wait for previous spm to complete
             "andi  r16,1           \n\t"
             "cpi r16,1             \n\t"
             "breq  wait_spm4       \n\t"
             "ldi r16,0x05          \n\t" //Write page pointed to by Z
             "sts %0,r16            \n\t"
             "spm                   \n\t"

             "wait_spm5:            \n\t"
             "lds r16,%0            \n\t" //Wait for previous spm to complete
             "andi  r16,1           \n\t"
             "cpi r16,1             \n\t"
             "breq  wait_spm5       \n\t"                  
             "ldi r16,0x11          \n\t" //Re-enable RWW section
             "sts %0,r16            \n\t"                  
             "spm                   \n\t"              

             "same_page:            \n\t"              
             "adiw  r30,2           \n\t" //Next word in FLASH
             "sbiw  r24,2           \n\t" //length-2
             "breq  final_write     \n\t" //Finished
             "rjmp  length_loop     \n\t"
             "final_write:          \n\t"
             "cpi r17,0             \n\t"
             "breq  block_done      \n\t"
             "adiw  r24,2           \n\t" //length+2, fool above check on length after short page write
             "rjmp  write_page      \n\t"
             "block_done:           \n\t"
             "clr __zero_reg__      \n\t" //restore zero register

             : "=m" (SPMCSR) : "M" (PAGE_SIZE) : "r0","r16","r17","r24","r25","r28","r29","r30","r31"
             );
          /* Should really add a wait for RWW section to be enabled, don't actually need it since we never */
          /* exit the bootloader without a power cycle anyhow */
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
        putch(SIG1);
        putch(SIG2);
        putch(SIG3);
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


void putch(char ch)
{
  while (!(UCSR0A & _BV(UDRE0)));
  UDR0 = ch;
}


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


void getNch(uint8_t count)
{
  uint8_t i;

  for (i=0; i<count; i++)
  {
    while (!(UCSR0A & _BV(RXC0)));
    UDR0;
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
    LED_PORT |= _BV(LED);
    _delay_ms(100);
    LED_PORT &= ~_BV(LED);
    _delay_ms(100);
  }
}


/* end of file ATmegaBOOT.c */
