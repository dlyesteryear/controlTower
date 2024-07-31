/*

 License GPLv3

 Based on
 https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB

*/

#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "HID.h"

extern const char *gp_serial;

class GamepadReport {
public:
  uint8_t reportId;
  uint8_t buttons[2]; // this includes the hat : 4 buttons, hat, 7 buttons
  uint8_t x;
  uint8_t y;
  GamepadReport();
};

typedef struct {
  uint8_t reportId;
  uint8_t strong, weak;
} RumbleReport;

class Gamepad_ : public PluggableUSBModule {
private:
  // uint8_t reportId;

protected:
  int getInterface(uint8_t *interfaceCount);
  int getDescriptor(USBSetup &setup);
  uint8_t getShortName(char *name);
  bool setup(USBSetup &setup);

  uint8_t epType[2];
  uint8_t protocol;
  uint8_t idle;

public:
  GamepadReport _GamepadReport;
  RumbleReport _RumbleReport;
  Gamepad_(void);
  void reset(void);
  void send();
  bool read();
};

#endif
