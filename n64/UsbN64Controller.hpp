#ifndef _USBN64CONTROLLER_HPP_
#define _USBN64CONTROLLER_HPP_
#include "UsbN64Controller.h"
#include "n64pad.hpp"
#include "packet.hpp"

#define HATSWITCH_UP 0x00
#define HATSWITCH_UPRIGHT 0x01
#define HATSWITCH_RIGHT 0x02
#define HATSWITCH_DOWNRIGHT 0x03
#define HATSWITCH_DOWN 0x04
#define HATSWITCH_DOWNLEFT 0x05
#define HATSWITCH_LEFT 0x06
#define HATSWITCH_UPLEFT 0x07
#define HATSWITCH_NONE 0x0F

#ifdef DEBUG_P
#define debugp(...) Serial1.print(__VA_ARGS__)
#else
#define debugp(...)
#endif

static const uint8_t hatMap[] PROGMEM = {
    HATSWITCH_NONE,      //----
    HATSWITCH_RIGHT,     //---R
    HATSWITCH_LEFT,      //--L-
    HATSWITCH_NONE,      //--LR
    HATSWITCH_DOWN,      //-D--
    HATSWITCH_DOWNRIGHT, //-D-R
    HATSWITCH_DOWNLEFT,  //-DL-
    HATSWITCH_DOWN,      //-DLR
    HATSWITCH_UP,        // U---
    HATSWITCH_UPRIGHT,   // U--R
    HATSWITCH_UPLEFT,    // U-L-
    HATSWITCH_UP,        // U-LR
    HATSWITCH_NONE,      // UD--
    HATSWITCH_RIGHT,     // UD-R
    HATSWITCH_LEFT,      // UDL-
    HATSWITCH_NONE       // LDRU
};

template <uint8_t P, uint8_t B>
UsbN64Controller<P, B>::UsbN64Controller()
    : connected(false), hasRumble(false) {}

template <uint8_t P, uint8_t B> void UsbN64Controller<P, B>::init() {
  gamepad.reset();
  N64Pad<P, B>::init();
  // delay(100);
  if (Packet<B>::pack(indata, 3 * 8)) {
    connected = true;
    if (indata[2] == 0x01) {
      debugp("has rumble\n");
      N64Pad<P, B>::initRumble();
      // delay(100);
      hasRumble = true;
    } else {
      debugp("no rumble\n");
    }
  }
}

static inline uint8_t clip(uint8_t ux) {
  int8_t x = (int8_t)ux;
  if (x < -80) {
    x = -80;
  } else if (x > 80) {
    x = 80;
  }
  return (uint8_t)x;
}

template <uint8_t P, uint8_t B> void UsbN64Controller<P, B>::loop() {

  if (connected) {
    N64Pad<P, B>::update();
    if (Packet<B>::pack(indata, 8 * 4)) {
      uint8_t b = indata[0];
      indata[0] = (b & 0xF0) | pgm_read_byte(hatMap + (b & 0x0F));
      indata[2] = clip(indata[2]);
      indata[3] = clip((uint8_t)(-((int8_t)(indata[3]))));

      void *gp = gamepad._GamepadReport.buttons;
      if (memcmp(gp, indata, sizeof(indata))) {
        memcpy(gp, indata, sizeof(indata));
        gamepad.send();
      }
    } else {
      connected = false;
      debugp("disconnected\n");
    }

    if (hasRumble && gamepad.read()) {
      debugp("setting rumble");
      debugp(gamepad._RumbleReport.weak, HEX);
      debugp(' ');
      debugp(gamepad._RumbleReport.strong, HEX);
      debugp("\n");
      N64Pad<P, B>::setRumble((gamepad._RumbleReport.weak != 0) ||
                              (gamepad._RumbleReport.strong != 0));
    }
  }
}

#endif
