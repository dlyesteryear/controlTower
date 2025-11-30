#ifndef _MMKEYBOARD_H_
#define _MMKEYBOARD_H_

#include "HID.h"

// extern const char *gp_serial;

typedef struct {
  uint8_t media;
} MMKeybReport;

class MMKeyboard_ : public PluggableUSBModule {
private:
  uint8_t reportId;

protected:
  int getInterface(uint8_t *interfaceCount);
  int getDescriptor(USBSetup &setup);
  uint8_t getShortName(char *name);
  bool setup(USBSetup &setup);

  uint8_t epType[1];
  uint8_t protocol;
  uint8_t idle;

public:
  MMKeybReport _MMKeybReport;
  MMKeyboard_(void);
  void reset(void);
  void send();
};

extern MMKeyboard_ MMKeyboard;

#endif
