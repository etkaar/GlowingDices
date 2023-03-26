/**
	Copyright (c) 2023 etkaar <https://github.com/etkaar>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
	OR OTHER DEALINGS IN THE SOFTWARE.
**/

/**
* ATtiny85
*
* There are rumors that ATiny85 would have only
* two pins capable of PWM; this is wrong. In fact
* there are three PWM capable pins.
*
* See page 2:
*   https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
*
* PB4 : OC1B : IC-PIN 3
* PB1 : OC0B : IC-PIN 6
* PB0 : OC0A : IC-PIN 5
*
* PB3 is not a PWM pin. The overline above it marks it as inverter pin (page 61).
*/
const byte PIN_RED = 4;
const byte PIN_GREEN = 1;
const byte PIN_BLUE = 0;

/**
* Brightnesses
*/
const byte INITIAL_BRIGHTNESS = 70;
const byte MAX_BRIGHTNESS = 255;
const byte MAX_BREATHING_BRIGHTNESS = 255;

/**
* Increment/decrement size for each loop run
*/
const byte INCREMENT_SIZE = 1;
const byte DECREMENT_SIZE = 1;

/**
* VARIABLES: Global
*/
byte program_state = 0;

unsigned long loop_latest_micros = micros();
unsigned long breathing_latest_micros = micros();

/**
* 0: Mode (0 = Increment, 1 = Decrement)
* 1: Current brightness
* 2: Microseconds before update (constantly updated)
*/
const byte MODE = 0;
const byte BRIGHTNESS = 1;
const byte MICROS = 2;

int pins[8][3] = {};

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop() {
  /**
  * INITIATE: First loop run
  */
  if (program_state == 0) {
      program_state = 1;

      delay(2500);

      /**
      * Gracefully turn on all LEDs
      */
      fadeIn(PIN_RED, 1, INITIAL_BRIGHTNESS, 5 * 1000);
      delay(350);

      fadeIn(PIN_GREEN, 1, INITIAL_BRIGHTNESS, 5 * 1000);
      delay(350);

      fadeIn(PIN_BLUE, 1, INITIAL_BRIGHTNESS, 5 * 1000);
      delay(350);
  }

  breathing(PIN_RED, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));
  breathing(PIN_GREEN, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));
  breathing(PIN_BLUE, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));
}

void breathing(byte pin_number, byte min_brightness, byte max_brightness, int microdelay) {
  /**
  * Microseconds that have passed since
  */
  int micros_passed = micros() - breathing_latest_micros;
  breathing_latest_micros = micros();

  /**
  * Always reduce the amount of microseconds remaining until next update
  */
  pins[pin_number][MICROS] = pins[pin_number][MICROS] - micros_passed;

  /**
  * Invoke update
  */
  if (pins[pin_number][MICROS] <= 0) {
    /**
    * Increment
    */
    if (pins[pin_number][MODE] == 0) {
      pins[pin_number][BRIGHTNESS] += INCREMENT_SIZE;
    /**
    * Decrement
    */
    } else {
      pins[pin_number][BRIGHTNESS] -= DECREMENT_SIZE;
    }

    /**
    * Cap upper and lower bounds
    */
    if (pins[pin_number][BRIGHTNESS] > max_brightness) {
      pins[pin_number][BRIGHTNESS] = max_brightness;
    }

    if (pins[pin_number][BRIGHTNESS] < min_brightness) {
      pins[pin_number][BRIGHTNESS] = min_brightness;
    }

    /**
    * Switch mode once limits have been exceeded
    */
    if (pins[pin_number][BRIGHTNESS] == min_brightness) {
      pins[pin_number][MODE] = 0;
    } else if (pins[pin_number][BRIGHTNESS] == max_brightness) {
      pins[pin_number][MODE] = 1;
    }

    /**
    * Change brightness
    */
    setIntensity(pin_number, pins[pin_number][BRIGHTNESS]);

    /**
    * Set microseconds remaining for next update
    */
    pins[pin_number][MICROS] = microdelay;
  }
}

void setIntensity(byte pin_number, byte value) {
  analogWrite(pin_number, value);
}

/**
* Effects
*/
void fadeIn(byte pin_number, byte start, byte end, int microdelay) {
  for (byte i = start; i < end; i++) {
    setIntensity(pin_number, i);
    delayMicroseconds(microdelay);
  }
}

void fadeOut(byte pin_number, byte start, byte end, int microdelay) {
  for (byte i = start; i > end; i--) {
    setIntensity(pin_number, i);
    delayMicroseconds(microdelay);
  }
}

void pulse(byte pin_number) {
  fadeIn(pin_number, pins[pin_number][BRIGHTNESS], MAX_BRIGHTNESS, 1 * 1000);

  fadeOut(pin_number, MAX_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, 1 * 1000);
  pins[pin_number][BRIGHTNESS] = MAX_BREATHING_BRIGHTNESS;
}
