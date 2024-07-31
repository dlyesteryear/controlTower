/*

 License GPLv3

 Based on
 https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB

*/

#include "Gamepad.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x04, // USAGE (Joystick)
    0xa1, 0x01, // Collection (Application)

    // Report ID for Output Report
    0x85, 0x01,       // Report ID 1
    0x09, 0x00,       // Usage (Undefined)
    0x95, 0x02,       // Report Count (2)
    0x75, 0x08,       // Report Size (8)
    0x91, 0x02,       // Output (Data, Variable, Absolute)
    0x15, 0xFF,       // Logical Maximum (255)
    0x26, 0xFF, 0x00, // Physical Maximum (255)

    0x85, 0x02, // Report ID 2
    // Hat Switch Input (4 positions: Left, Right, Up, Down)
    0x05, 0x01,       // Usage Page (Generic Desktop Controls)
    0x09, 0x39,       // Usage (Hat switch)
    0x15, 0x00,       // Logical Minimum (1)
    0x25, 0x07,       // Logical Maximum (7)
    0x46, 0x3B, 0x01, // Physical Maximum  : 315 degrees (Optional)
    0x75, 0x04,       // Report Size (4)
    0x95, 0x01,       // Report Count (1)
    0x65, 0x14,       // Unit              : English Rotation/Angular Position 1
                      // degree (Optional)
    0x81, 0x42,       // Input (Data, Variable, Absolute, Null State)

    // Button Controls
    0x05, 0x01, // Usage Page (Generic Desktop Controls)
    0x05, 0x09, // USAGE_PAGE (Button)
    0x19, 0x01, // USAGE_MINIMUM (Button 1)
    0x29, 0x0A, // USAGE_MAXIMUM (Button 10)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x95, 0x0A, // REPORT_COUNT (4/12)
    0x75, 0x01, // REPORT_SIZE (1)
    0x81, 0x02, // INPUT (Data,Var,Abs)

    0x95, 0x01, // REPORT_COUNT (1) - Padding bit
    0x75, 0x01, // REPORT_SIZE (1)
    0x81, 0x01, // INPUT (Const)

    // Rst
    0x05, 0x09, // USAGE_PAGE (Button)
    0x19, 0x0B, // USAGE_MINIMUM (Button 11)
    0x29, 0x0B, // USAGE_MAXIMUM (Button 11)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x81, 0x02, // INPUT (Data,Var,Abs)

    // Analog Joystick Input
    0x05, 0x01, // Usage Page (Generic Desktop Controls)
    0x09, 0x30, // Usage (X)
    0x09, 0x31, // Usage (Y)
    0x15, 0xB0, // Logical Minimum (-80)
    0x25, 0x50, // Logical Maximum (80)
    0x75, 0x08, // Report Size (8)
    0x95, 0x04, // Report Count (2)
    0x81, 0x02, // Input (Data, Variable, Absolute)

    0xc0

};

Gamepad_::Gamepad_(void)
    : PluggableUSBModule(2, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1) {
  epType[0] = EP_TYPE_INTERRUPT_IN;
  epType[1] = EP_TYPE_INTERRUPT_OUT;
  PluggableUSB().plug(this);
}

typedef struct {
  InterfaceDescriptor hid;
  HIDDescDescriptor desc;
  EndpointDescriptor in;
  EndpointDescriptor out;
} USBControllerHIDDescriptor;

int Gamepad_::getInterface(uint8_t *interfaceCount) {
  *interfaceCount += 1; // uses 1
  USBControllerHIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 2, USB_DEVICE_CLASS_HUMAN_INTERFACE,
                  HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptor)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT,
                 USB_EP_SIZE, 0x01),
      D_ENDPOINT(USB_ENDPOINT_OUT(pluggedEndpoint + 1),
                 USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0)

  };
  return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int Gamepad_::getDescriptor(USBSetup &setup) {
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
    return 0;
  }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
    return 0;
  }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) {
    return 0;
  }

  // Reset the protocol on reenumeration. Normally the host should not assume
  // the state of the protocol due to the USB specs, but Windows and Linux just
  // assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor,
                         sizeof(_hidReportDescriptor));
}

bool Gamepad_::setup(USBSetup &setup) {
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

GamepadReport::GamepadReport() {
  reportId = 2;
  buttons[0] = 0x0F; // null hat
  memset(buttons + 1, 0, sizeof(GamepadReport) - 2);
}

void Gamepad_::reset() {
  _GamepadReport = GamepadReport();
  this->send();
}

void Gamepad_::send() {

  USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport,
           sizeof(GamepadReport));
}

bool Gamepad_::read() {
  if (USB_Available(pluggedEndpoint + 1)) {
    return USB_Recv(pluggedEndpoint + 1, &_RumbleReport,
                    sizeof(RumbleReport)) == sizeof(RumbleReport);
  }
  return false;
}

uint8_t Gamepad_::getShortName(char *name) {
  if (!next) {
    strcpy(name, gp_serial);
    return strlen(name);
  }
  return 0;
}
