/*
 *  Based on the following
 *  NeoGeo Controller to USB
 *  Author: Mikael Norrgård <mick@daemonbite.com>
 *
 *  Copyright (c) 2020 Mikael Norrgård <http://daemonbite.com>
 *
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "Gamepad.h"
#include "MMKeyboard.h"
#include <USBAPI.h>

// 1=Diddly-squat-Delay-Debouncing™ activated, 0=Debounce deactivated
#define DEBOUNCE 1
#define DEBOUNCE_TIME 10 // Debounce time in milliseconds
//#define DEBUG            // Enables debugging (sends debug data to usb
// serial)

const char *gp_serial = "NeoGeo to USB";

Gamepad_ Gamepad;         // Set up USB HID gamepad
bool usbUpdate = false;   // Should gamepad data be sent to USB?
bool debounce = DEBOUNCE; // Debounce?
uint8_t pin;              // Used in for loops
uint32_t millisNow = 0;   // Used for Diddly-squat-Delay-Debouncing™

uint8_t axesDirect = 0x0f;
uint8_t axes = 0x0f;
uint8_t axesPrev = 0x0f;
uint8_t axesBits[4] = {0x10, 0x20, 0x40, 0x80};
uint32_t axesMillis[4];

uint32_t buttonsDirect = 0;
uint32_t buttons = 0;
uint32_t buttonsPrev = 0;

static const int BUTTONS_NUM = 19;

// PD
// PB
// PF | PE>>1 | PC

static const uint32_t SHIFT_D=0;
static const uint32_t SHIFT_B=8;
static const uint32_t SHIFT_F=16;
static const uint32_t SHIFT_E=15; //1 bit shifted
static const uint32_t SHIFT_C=16;

static const uint32_t buttonsSrcBits[BUTTONS_NUM] = {
    // conn 1 PD (6)  PB (2) PE (1)
    bit(SHIFT_D + 0), bit(SHIFT_D + 1), bit(SHIFT_D + 2), bit(SHIFT_D + 3),
    bit(SHIFT_B + 1), bit(SHIFT_D + 4), bit(SHIFT_E + 6), bit(SHIFT_B + 7),
    bit(SHIFT_D + 6),

    // conn 2  PB (5) PD (1) PC (2) PF (2)
    bit(SHIFT_B + 2), bit(SHIFT_B + 3), bit(SHIFT_B + 4), bit(SHIFT_B + 5),
    bit(SHIFT_B + 6), bit(SHIFT_D + 7), bit(SHIFT_F + 0), bit(SHIFT_F + 1),
    bit(SHIFT_C + 6), bit(SHIFT_C + 7)};

static const uint32_t VOLUME_UP = bit(0x18);
static const uint32_t VOLUME_DOWN = bit(0x19);

static const uint32_t buttonsDstBits[BUTTONS_NUM] = {
    bit(0x00), bit(0x01), bit(0x02), bit(0x03),  bit(0x04),
    bit(0x05), bit(0x06), bit(0x07), bit(0x08),  bit(0x09),
    bit(0x0A), bit(0x0B), bit(0x0C), bit(0x0D),  bit(0x0E),
    bit(0x0F), bit(0x10), VOLUME_UP, VOLUME_DOWN};

uint32_t buttonsMillis[BUTTONS_NUM];

#ifdef DEBUG
char buf[16];
uint32_t millisSent = 0;
#endif

static const uint8_t MASKB = 0b11111110;
static const uint8_t MASKC = 0b11000000;
static const uint8_t MASKD = 0b11011111;
static const uint8_t MASKE = 0b01000000;
static const uint8_t MASKF = 0b00000011; // only buttons, not axes

void setup() {

  // DIDR0 = 0xFF;
  // pinMode(A4, INPUT_PULLUP);
  // pinMode(A5, INPUT_PULLUP);

  // Buttons
  DDRB &= ~MASKB;
  PORTB |= MASKB;

  DDRC &= ~MASKC;
  PORTC |= MASKC;

  DDRD &= ~MASKD;
  PORTD |= MASKD;

  DDRE &= ~MASKE;
  PORTE |= MASKE;

  // Axes ad Buttons
  DDRF &= ~(0xF0 | MASKF); // Set A0-A3 as inputs
  PORTF |= (0xF0 | MASKF); // Enable internal pull-up resistors

  // Initialize debouncing timestamps
  for (pin = 0; pin < 4; pin++)
    axesMillis[pin] = 0;
  for (pin = 0; pin < BUTTONS_NUM; pin++)
    buttonsMillis[pin] = 0;

#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void loop() {
  while (!USBDevice.configured()) {
  }

  MMKeyboard.reset();
  Gamepad.reset();

  while (true) {

    // Get current time, the millis() function should take about 2µs to complete
    millisNow = millis();

    /*for (uint8_t i = 0; i < 10; i++){ // One iteration (when debounce is
       enabled) takes approximately 35µs
              // to complete, so we don't need to check the time between every
              // iteration*/
    // Read axis and button inputs (bitwise NOT results in a 1 when button/axis
    // pressed)
    axesDirect = ~(PINF & B11110000);
    buttonsDirect =
        ~((uint32_t)(PIND & MASKD) | (((uint32_t)(PINB & MASKB)) << SHIFT_B) |
          (((uint32_t)((PINF & MASKF) | (PINC & MASKC) | ((PINE & MASKE) >> 1)))
           << SHIFT_F));

    if (debounce) {
      // Debounce axes
      for (pin = 0; pin < 4; pin++) {
        // Check if the current pin state is different to the stored state and
        // that enough time has passed since last change
        if ((axesDirect & axesBits[pin]) != (axes & axesBits[pin]) &&
            (millisNow - axesMillis[pin]) > DEBOUNCE_TIME) {
          // Toggle the pin, we can safely do this because we know the current
          // state is different to the stored state
          axes ^= axesBits[pin];
          // Update the timestamp for the pin
          axesMillis[pin] = millisNow;
        }
      }

      // Debounce buttons
      for (pin = 0; pin < BUTTONS_NUM; pin++) {
        // Check if the current pin state is different to the stored state and
        // that enough time has passed since last change
        if (((buttonsDirect & buttonsSrcBits[pin]) == 0) !=
                ((buttons & buttonsDstBits[pin]) == 0) &&
            (millisNow - buttonsMillis[pin]) > DEBOUNCE_TIME) {
          // Toggle the pin, we can safely do this because we know the current
          // state is different to the stored state
          buttons ^= buttonsDstBits[pin];
          // Update the timestamp for the pin
          buttonsMillis[pin] = millisNow;
        }
      }
    } else {
      axes = axesDirect;
      buttons = 0;
      for (pin = 0; pin < BUTTONS_NUM; pin++) {
        if (buttonsSrcBits[pin] & buttonsDirect) {
          buttons |= buttonsDstBits[pin];
        }
      }
    }

    // Has axis inputs changed?
    if (axes != axesPrev) {
      // UP + DOWN = UP, SOCD (Simultaneous Opposite Cardinal Directions)
      // Cleaner
      if (axes & B10000000)
        Gamepad._GamepadReport.Y = -1;
      else if (axes & B01000000)
        Gamepad._GamepadReport.Y = 1;
      else
        Gamepad._GamepadReport.Y = 0;
      // UP + DOWN = NEUTRAL
      // Gamepad._GamepadReport.Y = ((axes & B01000000)>>6) - ((axes &
      // B10000000)>>7);
      // LEFT + RIGHT = NEUTRAL
      Gamepad._GamepadReport.X =
          ((axes & B00010000) >> 4) - ((axes & B00100000) >> 5);
      axesPrev = axes;
      usbUpdate = true;
    }

    uint32_t buttonChange = buttons ^ buttonsPrev;

    if (buttonChange) {
      Gamepad._GamepadReport.buttonsL = buttons & 0xFFFF;
      Gamepad._GamepadReport.buttonsH = (buttons >> 16) & 0xFF;
      buttonsPrev = buttons;
      usbUpdate = true;
    }

    // Should gamepad data be sent to USB?
    if (usbUpdate) {

      if (buttonChange & (VOLUME_UP | VOLUME_DOWN)) {
        MMKeyboard._MMKeybReport.media = buttons >> 24;
        MMKeyboard.send();
      } else {
        Gamepad.send();
      }
      usbUpdate = false;

#ifdef DEBUG
      sprintf(buf, "%06lu: %d%d%d%d", millisNow - millisSent,
              ((axes & 0x10) >> 4), ((axes & 0x20) >> 5), ((axes & 0x40) >> 6),
              ((axes & 0x80) >> 7));
      Serial.print(buf);
      sprintf(buf, " %lx", buttons);
      Serial.println(buf);
      millisSent = millisNow;
#endif
    }

  } // while true
}
