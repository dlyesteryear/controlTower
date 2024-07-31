#ifndef _USBPSXCONTROLLER_H_
#define _USBPSXCONTROLLER_H_

#include "Gamepad.h"
#include <PsxControllerBitBang.h>

template <uint8_t ATTPIN, uint8_t CMDPIN, uint8_t DATPIN, uint8_t CLKPIN>
class UsbPsxController
    : public PsxControllerBitBang<ATTPIN, CMDPIN, DATPIN, CLKPIN> {
  Gamepad_ gamepad;

public:
  void reset();
  void loop();
  void send();
  bool analogChanged();
  UsbPsxController();

private:
  boolean haveController, oldanalogValid;
  uint8_t oldlx, oldly, oldrx, oldry;
};

#endif
