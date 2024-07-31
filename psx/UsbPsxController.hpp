#include "UsbPsxController.h"

#define HATSWITCH_UP 0x000
#define HATSWITCH_UPRIGHT 0x010
#define HATSWITCH_RIGHT 0x020
#define HATSWITCH_DOWNRIGHT 0x030
#define HATSWITCH_DOWN 0x040
#define HATSWITCH_DOWNLEFT 0x050
#define HATSWITCH_LEFT 0x060
#define HATSWITCH_UPLEFT 0x070
#define HATSWITCH_NONE 0x0F0

static const uint8_t hatMap[] PROGMEM = {
    HATSWITCH_NONE,    //----
    HATSWITCH_UP,      //---U
    HATSWITCH_RIGHT,   //--R-
    HATSWITCH_UPRIGHT, //--RU

    HATSWITCH_DOWN,      //-D--
    HATSWITCH_NONE,      //-D-U
    HATSWITCH_DOWNRIGHT, //-DR-
    HATSWITCH_RIGHT,     //-DRU

    HATSWITCH_LEFT,   // L---
    HATSWITCH_UPLEFT, // L--U
    HATSWITCH_NONE,   // L-R-
    HATSWITCH_UP,     // L-RU

    HATSWITCH_DOWNLEFT, // LD--
    HATSWITCH_LEFT,     // LD-U
    HATSWITCH_DOWN,     // LDR-
    HATSWITCH_NONE      // LDRU
};

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
UsbPsxController<ATTPIN, CMDPIN, DATPIN, CLKPIN>::UsbPsxController()
    : haveController(false), oldanalogValid(false) {
  reset();
}

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
void UsbPsxController<ATTPIN, CMDPIN, DATPIN, CLKPIN>::reset() {
  gamepad.reset();
}

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
void UsbPsxController<ATTPIN, CMDPIN, DATPIN, CLKPIN>::send() {
  gamepad.send();
}

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
bool UsbPsxController<ATTPIN, CMDPIN, DATPIN, CLKPIN>::analogChanged() {
  bool changed = false;

  if (!this->analogSticksValid) {
    if (oldanalogValid) {
      oldlx = this->lx = oldly = this->ly = oldrx = this->rx = oldry =
          this->ry = ANALOG_IDLE_VALUE;
      oldanalogValid = false;
      changed = true;
    }
  } else {
    if ((oldlx != this->lx) || (oldly != this->ly) || (oldrx != this->rx) ||
        (oldry != this->ry)) {
      oldlx = this->lx;
      oldly = this->ly;
      oldrx = this->rx;
      oldry = this->ry;
      changed = true;
    }
  }
  oldanalogValid = this->analogSticksValid;
  return changed;
}

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
void UsbPsxController<ATTPIN, CMDPIN, DATPIN, CLKPIN>::loop() {
  if (!haveController) {
    if (this->begin()) {
      if (this->enterConfigMode()) {
        this->enableRumble();
        this->exitConfigMode();
      }
      haveController = true;
    }
  } else { // haveController

    if (gamepad.read()) { // usb->psx
      this->setRumble(
          gamepad._RumbleReport.weak != 0,
          gamepad._RumbleReport.strong); // effect on the following read
    }

    if (!this->read()) { // psx->usb
      haveController = false;
    } else {
      bool tosend = false;
      if (this->buttonsChanged()) {
        uint16_t b = this->getButtonWord();
        gamepad._GamepadReport.buttons =
            (b & 0xFF0F) | pgm_read_byte(hatMap + ((b >> 4) & 0x0F));

        tosend = true;
      }

      if (this->analogChanged()) {
        gamepad._GamepadReport.lx = this->lx;
        gamepad._GamepadReport.ly = this->ly;
        gamepad._GamepadReport.rx = this->rx;
        gamepad._GamepadReport.ry = this->ry;
        tosend = true;
      }
      if (tosend) {
        gamepad.send();
      }
    } // this->read()
  }   // haveController
} // loop
