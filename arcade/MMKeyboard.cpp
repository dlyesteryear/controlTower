#include "MMKeyboard.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
    // USAGE_PAGE (Consumer Devices) - Better than "Generic Desktop" for media
    // keys
    // 0x05, 0x0C,
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    // USAGE (Consumer Control) - Better than "Keyboard" for a volume knob/pad
    // 0x09, 0x01,
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)

    // --- Media Keys (Volume Up, Volume Down) ---
    0x05, 0x0C, // USAGE_PAGE (Consumer Devices)
    0x09, 0xE9, // USAGE (Volume Up)
    0x09, 0xEA, // USAGE (Volume Down)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1) - 1 bit per key
    0x95, 0x02, // REPORT_COUNT (2) - 2 keys
    0x81, 0x02, // INPUT (Data,Var,Abs)

    // --- PADDING (CRITICAL FIX) ---
    // You have used 2 bits. USB sends data in bytes (8 bits).
    // You must pad the remaining 6 bits to complete the byte.
    0x75, 0x06, // REPORT_SIZE (6)
    0x95, 0x01, // REPORT_COUNT (1)
    0x81, 0x03, // INPUT (Cnst,Var,Abs) - "Cnst" means constant/padding

    0xC0 // END_COLLECTION
};

static const char *gp_serial_keyb = "Volume keys";

MMKeyboard_::MMKeyboard_(void) : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1)
{
  epType[0] = EP_TYPE_INTERRUPT_IN;
  PluggableUSB().plug(this);
}

int MMKeyboard_::getInterface(uint8_t* interfaceCount)
{
  *interfaceCount += 1; // uses 1
  HIDDescriptor hidInterface = {
    D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
    D_HIDREPORT(sizeof(_hidReportDescriptor)),
    D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
  };
  return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}


int MMKeyboard_::getDescriptor(USBSetup& setup)
{
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) { return 0; }

  // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
  // due to the USB specs, but Windows and Linux just assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
}

bool MMKeyboard_::setup(USBSetup &setup) {
  if (pluggedInterface != setup.wIndex) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
    if (request == HID_GET_REPORT) {
      // TODO: HID_GetReport();
      return true;
    }
    if (request == HID_GET_PROTOCOL) {
      // TODO: Send8(protocol);
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }
    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }
    if (request == HID_SET_REPORT) {
    }
  }

  return false;
}

void MMKeyboard_::reset() {
  _MMKeybReport.media = 0;
  this->send();
}

void MMKeyboard_::send() {
  USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_MMKeybReport,
           sizeof(_MMKeybReport));
}

uint8_t MMKeyboard_::getShortName(char *name) {
  if (!next) {
    strcpy(name, gp_serial_keyb);
    return strlen(name);
  }
  return 0;
}

MMKeyboard_ MMKeyboard;
