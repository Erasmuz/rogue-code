#include <rDuinoLEDs.h>

#define DELAY 40

void setup()
{
}

void loop()
{
  int i;

  for (i = 0; i < 7; i++)
  {
    LEDS.set(i+1, HIGH);
    LEDS.set(i, LOW);
    delay(DELAY);
  }
  for (i = 7; i > 0; i--)
  {
    LEDS.set(i-1, HIGH);
    LEDS.set(i, LOW);
    delay(DELAY);
  }
}
