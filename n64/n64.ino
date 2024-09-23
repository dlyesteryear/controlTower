/*

 License GPLv3

 Based on
 https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB

*/

#include "UsbN64Controller.hpp"
#include <stdbool.h>

uint8_t recvbuffer[RECVBUFFER_SIZE];
uint8_t sendbuffer[SENDBUFFER_SIZE];

const char *gp_serial = "N64 to USB";

UsbN64Controller<'D', 0> pad1;
UsbN64Controller<'D', 1> pad2;

void setup() {
  PORTD &= ~0x03;
  DDRD &= ~0x03;
  pad1.init();
  pad2.init();
  delay(10);
}

void loop() {
  for (int i = 0; i < 10; ++i) {
    pad1.gamepad.send();
    delay(50);
    pad2.gamepad.send();
    delay(50);
  };
  while (true) {
    if (!pad1.connected &&
        !pad2.connected) { // trying to init one slows for 1 timeout. If none is
                           // there, you're slowing noone
      pad1.init();
      pad2.init();
    }
    pad1.loop();
    pad2.loop();
  }
}
