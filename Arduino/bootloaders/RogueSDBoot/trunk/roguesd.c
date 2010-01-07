

#include <avr/pgmspace.h>
#include <ctype.h>
#include "usart.h"


int16_t _comm_buff;
uint8_t LastErrorCode;
uint8_t _promptchar;
uint16_t _fwversion;

enum moduletype {uMMC = 1, uMP3, rMP3} _moduletype;

// Constants
#define UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS  10201
#define UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS  11101


// PGMSPACE stuff
#define PSTRU(s) (__extension__({static unsigned char __c[] PROGMEM = (s); &__c[0];}))

const prog_uchar FCstr_P[] = "FC";

// Set our PGMSPACE read byte routines - far for large devices
#if defined(RAMPZ)
#define P_read_byte(a) pgm_read_byte_far(a)
#else
#define P_read_byte(a) pgm_read_byte(a)
#endif


// Prototypes
void rogueSDCloseAll(void);

#define _comm_available()  usartHasData()


uint8_t _comm_read(void)
{
  uint8_t r;

  if (_comm_buff >= 0)
  {
    r = _comm_buff;
    _comm_buff = -1;
  }
  else
  {
    r = usartGetByte();
  }

  return r;
}


uint8_t _comm_peek(void)
{
  uint8_t r;

  if (_comm_buff >= 0)
  {
    r = _comm_buff;
  }
  else
  {
    r = usartGetByte();
    _comm_buff = r;
  }

  return r;
}


#define _comm_write(c)  usartSendByte(c)


int8_t _read_blocked(void)
{
  // int8_t r;
  
  if (_comm_buff < 0)
    while (!_comm_available());

  return _comm_read();
}


void printstr_P(const prog_uchar *str)
{
  register uint8_t c;

  while ((c = P_read_byte(str)))
  {
    _comm_write(c);
    str++;
  }
}


int32_t _getnumber(uint8_t base)
{
	uint8_t c, neg = 0;
	uint32_t val;

	val = 0;
	while(!_comm_available());
  c = _comm_peek();
  
  if(c == '-')
  {
    neg = 1;
    _comm_read();  // remove
    while(!_comm_available());
    c = _comm_peek();
  }
  
	while(((c >= 'A') && (c <= 'Z'))
	    || ((c >= 'a') && (c <= 'z'))
	    || ((c >= '0') && (c <= '9')))
	{
		if(c >= 'a') c -= 0x57;             // c = c - 'a' + 0x0a, c = c - ('a' - 0x0a)
		else if(c >= 'A') c -= 0x37;        // c = c - 'A' + 0x0A
		else c -= '0';
		if(c >= base) break;

		val *= base;
		val += c;
		_comm_read();                       // take the byte from the queue
		while(!_comm_available());          // wait for the next byte
		c = _comm_peek();
	}
	return neg ? -val : val;
}


int8_t _get_response(void)
{
  // looking for a response
  // If we get a space " ", we return as good and the remaining data can be retrieved
  // " ", ">", "Exx>" types only
  uint8_t r;
  uint8_t resp = 0;

  // we will return 0 if all is good, error code otherwise

  r = _read_blocked();

  if(r == ' ' || r == _promptchar)
    resp = 0;

  else if(r == 'E')
  {
    LastErrorCode = _getnumber(16);     // get our error code
    _read_blocked();                    // consume prompt
    
    resp = -1;
  }
  
  else
  {
    LastErrorCode = 0xFF;               // something got messed up, a resync would be nice
    resp = -1;
  }
  
  return resp;
}


int16_t _get_version(void)
{
  // get the version, and module type
  _comm_write('V'); _comm_write('\r');
  
  // Version format: mmm.nn[-bxxx] SN:TTTT-ssss...
  
  // get first portion mmm.nn
  _fwversion = _getnumber(10);
  _read_blocked();                      // consume '.'
  _fwversion *= 100;
  _fwversion += _getnumber(10);
  // ignore beta version (-bxxx), if it's there
  if (_read_blocked() == '-')
  {
    for (char i = 0; i < 4; i++)
      _read_blocked();
  }
  // otherwise, it was a space

  // now drop the SN:
  _read_blocked();
  _read_blocked();
  _read_blocked();

  if (_read_blocked() == 'R')
    _moduletype = rMP3;
  else
  {
    // either UMM1 or UMP1
    // so drop the M following the U
    _read_blocked();
    if (_read_blocked() == 'M')
      _moduletype = uMMC;
    else
      _moduletype = uMP3;
  }

  // ignore the rest
  while (_read_blocked() != '-');

  // consume up to and including prompt
  while (isalnum(_read_blocked()));
  
  return _fwversion;
}






int8_t rogueSDSync(void)
{
  // procedure:
  // 1. sync (send ESC, clear prompt)
  // 2. get version ("v"), and module type
  // 3. change settings as needed
  // 4. check status
  // 5. close files (if needed - E08, or other error, not needed)

  // 0. empty any data in the serial buffer
  _comm_buff = -1;

  // 1. sync
  _comm_write(0x1b);                    // send ESC to clear buffer on uMMC
  _read_blocked();                      // consume prompt

  // 2. get version (ignore prompt - just drop it)
  
  _get_version();

  // 3. change settings as needed
  // OLD: write timeout setting = 10 ms timeout
  // NEW: listing style = old style (0)


  if ((_moduletype == uMMC && _fwversion < UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS) ||
      (_moduletype == uMP3 && _fwversion < UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS))
  {
    // we need to set the write timeout to allow us to control when a line is finished
    // in writeln.
//    changesetting('1', 1);              // 10 ms timeout
  }
  else
  {
    // we're using the new version
    // Let's make sure the Listing Style setting is set to the old style
//    if (getsetting('L') != 0)
//    {
//      changesetting('L', 0);
//    }

    // get the prompt char
    _comm_write('S');
    if (_moduletype != uMMC) { _comm_write('T'); };
    _comm_write('P'); _comm_write('\r');  // get our prompt (if possible)
    _promptchar = _getnumber(10);
    _read_blocked();                    // consume prompt
  }

 
  // 4. check status
//  if (_moduletype != uMMC) { printstr_P(PSTRU("FC")); };
  if (_moduletype != uMMC) { printstr_P(FCstr_P); };
  _comm_write('Z'); _comm_write('\r');  // Get status

  if (_get_response())
    return -1;
  else
  {
    // good
    _read_blocked();                    // consume prompt

    // 5. close all files
    rogueSDCloseAll();                  // ensure all handles are closed

    return 0;
  }
}


void rogueSDCloseAll(void)
{
  if ((_moduletype == uMMC && _fwversion < UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS) ||
      (_moduletype == uMP3 && _fwversion < UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS))
  {
    // old
    for(uint8_t i=1; i<=4; i++)
    {
      if (_moduletype != uMMC) { printstr_P(FCstr_P); };
      _comm_write('C'); _comm_write('0'+i); _comm_write('\r');
      _get_response();
    }
  }
  else
  {
    // new
    if (_moduletype != uMMC) { printstr_P(FCstr_P); };
    _comm_write('C'); _comm_write('\r');
    _read_blocked();                    // consume prompt
  }
}


// private
int8_t rogueSDOpen(const prog_uchar *filename)
{
  int8_t resp;

  if (_moduletype != uMMC) { printstr_P(FCstr_P); };
  printstr_P(PSTRU("O1 R "));
//  _comm_write('O'); _comm_write('1'); _comm_write(' '); _comm_write('R'); _comm_write(' ');

  printstr_P(filename);

  _comm_write('\r');

  resp = _get_response();
  return resp < 0 ? resp : 1;
}


int16_t rogueSDReadln(uint8_t *tostr)
{
  int8_t r, i;
  
  if ((_moduletype == uMMC && _fwversion < UMMC_MIN_FW_VERSION_FOR_NEW_COMMANDS) ||
      (_moduletype == uMP3 && _fwversion < UMP3_MIN_FW_VERSION_FOR_NEW_COMMANDS))
  {
    return -1;
  }
  
  if (_moduletype != uMMC) { printstr_P(FCstr_P); };
  printstr_P(PSTRU("RL1 48\r"));                       // Read a line, maxlength chars, maximum

  if(_get_response())
  {
    if(LastErrorCode == 7)  // EOF
      return -1;
    else
      return -2;
  }
  
  // otherwise, read the data

  i = 0;
  r = _read_blocked();

  while(r != _promptchar)               // we could have a blank line
  {
    tostr[i++] = r;
    r = _read_blocked();
  }

  tostr[i] = 0;                         // terminate our string

  return i;
}
