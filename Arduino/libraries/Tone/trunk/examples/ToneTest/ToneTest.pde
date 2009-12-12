// This example plays notes 'a' through 'g' sent over the Serial Monitor.
// 's' stops the current playing tone.

// NOTE: This ONLY plays on pin 11 on the ATmega168 and ATmega328
//       (or ONLY on pin 10 on the ATmega1280)
// PWM will be disabled for pins 3 and 11 ('168, '328)
// (or pins 9 and 10 on the '1280)

#include <Tone.h>

int notes[] = { NOTE_A3,
                NOTE_B3,
                NOTE_C4,
                NOTE_D4,
                NOTE_E4,
                NOTE_F4,
                NOTE_G4 };

void setup(void)
{
  Serial.begin(9600);
}

void loop(void)
{
  char c;

  if(Serial.available())
  {
    c = Serial.read();
    
    switch(c)
    {
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
        Tone::play(notes[c - 'a']);
        Serial.println(notes[c - 'a']);
        break;
      case 's':
        Tone::stop();
        break;
      default:
        Tone::play(NOTE_D2);
        delay(300);
        Tone::stop();
        delay(100);
        Tone::play(NOTE_D2);
        delay(300);
        Tone::stop();
        break;
    }
  }
}
