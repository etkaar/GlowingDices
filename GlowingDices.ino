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
* PWM
*
*  There are rumors that ATiny85 would have only
*  two pins capable of PWM; this is wrong. In fact
*  there are three PWM capable pins.
*
*  See page 2:
*    https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
*
*  IC-PIN 1 : PB5 = Unused
*  IC-PIN 2 : PB3 = Input, Switch #2
*  IC-PIN 3 : PB4 = Output, PWM (Red)
*  IC-PIN 4 : GND = GROUND

*  IC-PIN 5 : PB0 = Output, PWM (Blue)
*  IC-PIN 6 : PB1 = Output, PWM (Green)
*  IC-PIN 7 : PB2 = Input, Switch #1
*  IC-PIN 8 : VCC = +5 VDC
*
*  PB3 is not a PWM pin. The overline above it marks it as inverter pin (page 61).
*/
const byte NUMBER_OF_LEDS = 3;

const byte PIN_RED = 4;
const byte PIN_GREEN = 1;
const byte PIN_BLUE = 0;

const byte LED_PINS_LIST[NUMBER_OF_LEDS] = {PIN_RED, PIN_GREEN, PIN_BLUE};

/*
*  PB2 and PB3 are our input pins. From there we will
*  detect state changes by mechanicalal switches.
*/
const byte PIN_SWITCH_ONE = 2;
const byte PIN_SWITCH_TWO = 3;

/**
* Brightnesses
*/
const byte INITIAL_BRIGHTNESS = 70;
const byte MAX_BRIGHTNESS = 255;
const byte MAX_BREATHING_BRIGHTNESS = 255;

/**
* PWM increment/decrement size for each loop run
*/
const byte INCREMENT_SIZE = 1;
const byte DECREMENT_SIZE = 1;

/**
* VARIABLES: Global
*/
byte program_state = 0;

unsigned long effects_latest_millis = millis();
unsigned long effects_invoke_count = 0;

unsigned long breathing_latest_micros = micros();

/**
* SWITCHES
*
* 0: Pin number
* 1: Current state
* 2: State before current state (loop check)
*/
const byte NUMBER_OF_SWITCHES = 2;

const byte SWITCH_ONE = 0;
const byte SWITCH_TWO = 1;

const byte PIN_NUMBER = 0;
const byte CURRENT_STATE = 1;
const byte STATE_BEFORE = 2;
const byte HAS_BEEN_TOGGLED = 3;

byte switches[NUMBER_OF_SWITCHES][4] = {
  {PIN_SWITCH_ONE, false, false, false},
  {PIN_SWITCH_TWO, false, false, false}
};

/**
* PINS
*
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

  for (byte i = 0; i < NUMBER_OF_SWITCHES; i++) {
    pinMode(switches[i][PIN_NUMBER], INPUT);
  }
}

void loop() {
  /**
  * UPDATE: Switch states
  */
  bool has_any_switch_been_toggled = false;
  bool is_any_switch_on = false;

  for (byte i = 0; i < NUMBER_OF_SWITCHES; i++) {
    switches[i][CURRENT_STATE] = false;
    switches[i][HAS_BEEN_TOGGLED] = false;

    if (digitalRead(switches[i][PIN_NUMBER]) == LOW) {
      switches[i][CURRENT_STATE] = true;
      is_any_switch_on = true;
    }

    if (switches[i][STATE_BEFORE] != switches[i][CURRENT_STATE]) {
      switches[i][HAS_BEEN_TOGGLED] = true;
      has_any_switch_been_toggled = true;
    }

    switches[i][STATE_BEFORE] = switches[i][CURRENT_STATE];
  }

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
  } else {

    /**
    * EFFECTS: Constant breathing
    */
    breathing(PIN_RED, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));
    breathing(PIN_GREEN, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));
    breathing(PIN_BLUE, INITIAL_BRIGHTNESS, MAX_BREATHING_BRIGHTNESS, random(100, 6000));

    /**
    * EFFECT: Immediately after toggle to show it was successful
    */
    if (has_any_switch_been_toggled && is_any_switch_on) {
      flash(LED_PINS_LIST, NUMBER_OF_LEDS, 50);

      /**
      * Reset to prevent immediate effects after changing state
      */
      effects_latest_millis = millis();
    }

    /**
    * EFFECTS: Periodically if any switch is on
    */
    if (is_any_switch_on) {
      int timeout;

      if (switches[SWITCH_ONE][CURRENT_STATE]) {
        timeout = 7000;
      } else if (switches[SWITCH_TWO][CURRENT_STATE]) {
        timeout = 31000;
      }

      if (millis() - effects_latest_millis > timeout) {
        /**
        * Switch #1: Pulses
        * Switch #2: Flashes
        */
        if (effects_invoke_count % 4 == 0) {
          for (byte i = 0; i < 6; i++) {
            flash(LED_PINS_LIST, NUMBER_OF_LEDS, 50);
          }
        } else {
          for (byte i = 0; i < 3; i++) {
            pulse(LED_PINS_LIST, NUMBER_OF_LEDS);
          }
        }

        effects_latest_millis = millis();
        effects_invoke_count += 1;
      }
    }
  }
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

void pulse(byte pin_numbers[], byte size) {
  for (byte i = 0; i < size; i++) {
    fadeIn(pin_numbers[i], pins[pin_numbers[i]][BRIGHTNESS], MAX_BRIGHTNESS, 1 * 1000);

    fadeOut(pin_numbers[i], MAX_BRIGHTNESS, INITIAL_BRIGHTNESS, 1 * 1000);
    pins[pin_numbers[i]][BRIGHTNESS] = INITIAL_BRIGHTNESS;
  }
}

void flash(byte pin_numbers[], byte size, byte end_millidelay) {
  for (byte i = 0; i < size; i++) {
    setIntensity(pin_numbers[i], MAX_BRIGHTNESS);
    delay(120);

    setIntensity(pin_numbers[i], INITIAL_BRIGHTNESS);
    pins[pin_numbers[i]][BRIGHTNESS] = INITIAL_BRIGHTNESS;

    if (i > 0) {
      delay(end_millidelay);
    }
  }
}

