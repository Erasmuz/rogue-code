# Arduino/Wiring SoftPWM Library #


---





---


## What's New? ##

Version 0005:

  * Fixed Arduino 1.0 mess.
  * Added Polarity control.
  * Up to 20 channels can be defined.

## Download ##

**[SoftPWM Library - Latest Version](http://rogue-code.googlecode.com/files/Wiring-Library-SoftPWM-V0005.zip)**

## Installation ##

See: InstallingArduinoLibraries

## Description ##

An Arduino and Wiring Library to produce PWM signals on any arbitrary pin.

It was originally designed for use controlling the brightness of LEDs, but could be modified to control servos and other low frequency PWM controlled devices as well.

It uses a single hardware timer (Timer 2) on the microcontroller to generate up to 20 PWM channels.

## Features ##

  * Arbitrary output pins.
  * Up to 20 different channels can be created.
  * True zero level, i.e. off = off
  * Separate fade rates for on and off.


**Notes:**

This library hasn't been tested with servos, so beware that it may not work as you may expect if used to control servos.


## Usage Example ##

```
#include <SoftPWM.h>

void setup()
{
  // Initialize
  SoftPWMBegin();

  // Create and set pin 13 to 0 (off)
  SoftPWMSet(13, 0);

  // Set fade time for pin 13 to 100 ms fade-up time, and 500 ms fade-down time
  SoftPWMSetFadeTime(13, 100, 500);
}

void loop()
{
  // Turn on - set to 100%
  SoftPWMSetPercent(13, 100);

  // Wait for LED to turn on - you could do other tasks here
  delay(100);

  // Turn off - set to 0%
  SoftPWMSetPercent(13, 0);

  // Wait for LED to turn off
  delay(500);
}
```


## Function Descriptions ##

`SoftPWMBegin([`_**`defaultPolarity`**_`])`
  * Initializes the library - sets up the timer and other tasks.
  * optional _**`defaultPolarity`**_ allows all newly defined pins to take on this polarity.
    * Values: **`SOFTPWM_NORMAL`**, **`SOFTPWM_INVERTED`**

`SoftPWMSet(`_**`pin`**_`, `_**`value`**_`)`
  * _**`pin`**_ is the output pin.
  * _**`value`**_ is a value between 0 and 255 (inclusive).

`SoftPWMSetPercent(`_**`pin`**_`, `_**`percent`**_`)`
  * _**`pin`**_ is the output pin.
  * _**`percent`**_ is a value between 0 and 100 (inclusive).

`SoftPWMSetFadeTime(`_**`pin`**_`, `_**`fadeUpTime`**_`, `_**`fadeDownTime`**_`)`
  * _**`pin`**_ is the output pin.
  * _**`fadeuptime`**_ is the time in milliseconds that it will take the channel to fade from 0 to 255.
    * Range: 0 to 4000
  * _**`fadedowntime`**_ is the time in milliseconds that it will take the channel to fade from 255 to 0.
    * Range: 0 to 4000

`SoftPWMSetPolarity(`_**`pin`**_`, `_**`polarity`**_`)`
  * _**`pin`**_ is the output pin.
  * _**`polarity`**_ is the polarity for the given pin.

### Notes ###

  * You can use `ALL` in place of the pin number to have the function act on all currently set channels.
    * e.g. `SoftPWMSetFadeTime(ALL, 100, 400)` - this will set all created channels to have a fade-up time of 100 ms and a fade-down time of 400.
  * The polarity setting of the pin is as follows:
    * **`SOFTPWM_NORMAL`** means that the pin LOW when the PWM value is 0, whereas **`SOFTPWM_INVERTED`** indicates the pin should be HIGH when the PWM value is 0.

## Demonstrations ##


---


Arduino Duemilanove LED Blink example - available as library example:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=9tTd7aLm9aQ' target='_blank'><img src='http://img.youtube.com/vi/9tTd7aLm9aQ/0.jpg' width='425' height=344 /></a>


---


rDuino LEDHead Bounce example - available as library example:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=jE7Zw1zNL6c' target='_blank'><img src='http://img.youtube.com/vi/jE7Zw1zNL6c/0.jpg' width='425' height=344 /></a>


---


More demos:

http://www.youtube.com/view_play_list?p=33BB5D2E20609C52


---
