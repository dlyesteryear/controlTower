/*

 License GPLv3

 Based on
 https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB
 https://github.com/hosterholz/DaemonBite-Retro-Controllers-USB

*/
#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "HID.h"

extern const char *gp_serial;

typedef struct {
  uint8_t reportId;
  uint16_t buttons; // this includes the hat : 4 buttons, hat, 8 buttons

  uint8_t lx;
  uint8_t ly;
  uint8_t rx;
  uint8_t ry;
} GamepadReport;

typedef struct {
  uint8_t reportId;
  uint8_t strong, weak;
} RumbleReport;

class Gamepad_ : public PluggableUSBModule {
private:
  uint8_t reportId;

protected:
  int getInterface(uint8_t *interfaceCount);
  int getDescriptor(USBSetup &setup);
  uint8_t getShortName(char *name);
  bool setup(USBSetup &setup);

  uint8_t epType[2];
  uint8_t protocol;

public:
  uint8_t idle;
  GamepadReport _GamepadReport;
  RumbleReport _RumbleReport;
  Gamepad_(void);
  void reset(void);
  void send();
  bool read();
};

#endif
