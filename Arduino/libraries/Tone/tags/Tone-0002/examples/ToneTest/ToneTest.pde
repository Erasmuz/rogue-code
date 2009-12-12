// Duelling Tones - Simultaneous tone generation.
// To mix the output of the signals to output to a small speaker (i.e. 8 Ohms or higher),
// simply use 1K Ohm resistors from each output pin and tie them together at the speaker.
// Don't forget to connect the other side of the speaker to ground!

// This example plays notes 'a' through 'g' sent over the Serial Monitor.
// 's' stops the current playing tone.  Use uppercase letters for the second.

#include <Tone.h>

int notes[] = { NOTE_A3,
                NOTE_B3,
                NOTE_C4,
                NOTE_D4,
                NOTE_E4,
                NOTE_F4,
                NOTE_G4 };

Tone np1(11);
Tone np2(12);

void setup(void)
{
  Serial.begin(9600);
  np1.begin();
  np2.begin();
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
        np1.play(notes[c - 'a']);
        Serial.println(notes[c - 'a']);
        break;
      case 's':
        np1.stop();
        break;

      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
        np2.play(notes[c - 'A']);
        Serial.println(notes[c - 'A']);
        break;
      case 'S':
        np2.stop();
        break;

      default:
        np2.stop();
        np1.play(NOTE_B2);
        delay(300);
        np1.stop();
        delay(100);
        np2.play(NOTE_B2);
        delay(300);
        np2.stop();
        break;
    }
  }
}
