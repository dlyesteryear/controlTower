/*

 License GPLv3

 Based on
 https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB
 https://github.com/hosterholz/DaemonBite-Retro-Controllers-USB

*/

#include "UsbPsxController.hpp"

// ATT: 20 chars max (including NULL at the end) according to Arduino source
// code. Additionally serial number is used to differentiate arduino projects to
// have different button maps!

const char *gp_serial = "PSX to USB";

// 18 is A0
#define PIN_PS2_DAT_1 18
#define PIN_PS2_CMD_1 2
#define PIN_PS2_ATT_1 3
#define PIN_PS2_CLK_1 4
#define PIN_PS2_ACK_1 19

#define PIN_PS2_DAT_2 20
#define PIN_PS2_CMD_2 8
#define PIN_PS2_ATT_2 9
#define PIN_PS2_CLK_2 10
#define PIN_PS2_ACK_2 21

UsbPsxController<PIN_PS2_ATT_1, PIN_PS2_CMD_1, PIN_PS2_DAT_1, PIN_PS2_CLK_1>
    psx1;
UsbPsxController<PIN_PS2_ATT_2, PIN_PS2_CMD_2, PIN_PS2_DAT_2, PIN_PS2_CLK_2>
    psx2;

void setup() {}

void loop() {
  for (int i = 0; i < 10; ++i) {
    psx1.gamepad.send();
    delay(50);
    psx2.gamepad.send();
    delay(50);
  };
  while (true) {
    psx1.loop();
    psx2.loop();
  }
}

/* -------------------------------------------------------------------------
PSX controller socket (looking face-on at the front of the socket):
_________________________
| 9 8 7 | 6 5 4 | 3 2 1 |
\_______________________/

PSX controller plug (looking face-on at the front of the controller plug):
_________________________
| 1 2 3 | 4 5 6 | 7 8 9 |
\_______________________/

PSX       Arduino Pro Micro
--------------------------------------
1 DATA    A0  PF7
2 CMD     2   PD1
3 +7.6V
4 GND     GND
5 VCC     VCC
6 ATT     3   PD0
7 CLK     4   PD4
8 N/C
9 ACK

------------------------------------------------------------------------- */
