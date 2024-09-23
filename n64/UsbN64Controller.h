#ifndef _USBN64CONTROLLER_H_
#define _USBN64CONTROLLER_H_

#include "Gamepad.h"
#include "n64pad.h"

template <uint8_t P, uint8_t B> class UsbN64Controller : public N64Pad<P, B> {
public:
  Gamepad_ gamepad;
  void init();
  void loop();
  bool connected;
  UsbN64Controller();

private:
  uint8_t indata[4];
  bool hasRumble;
};

#endif
