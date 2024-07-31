#ifndef _N64PAD_HPP
#define _N64PAD_HPP

#include "n64pad.h"
#include "packet.hpp"

template <uint8_t P, uint8_t B> void N64Pad<P, B>::init() {
  transaction(Packet<B>::CONTROLLER_RESET, sizeof(Packet<B>::CONTROLLER_RESET),
              3 * 8);
}

template <uint8_t P, uint8_t B> void N64Pad<P, B>::initRumble() {
  transaction(Packet<B>::RUMBLE_INIT, sizeof(Packet<B>::RUMBLE_INIT), 1 * 8);
}

template <uint8_t P, uint8_t B> void N64Pad<P, B>::update() {
  transaction(Packet<B>::CONTROLLER_STATE, sizeof(Packet<B>::CONTROLLER_STATE),
              4 * 8);
}

template <uint8_t P, uint8_t B> void N64Pad<P, B>::setRumble(bool b) {
  if (b) {
    transaction(Packet<B>::RUMBLE_ON, sizeof(Packet<B>::RUMBLE_ON), 1 * 8);
  } else {
    transaction(Packet<B>::RUMBLE_OFF, sizeof(Packet<B>::RUMBLE_OFF), 1 * 8);
  }
}

template <uint8_t P, uint8_t B>
void N64Pad<P, B>::transaction(const uint8_t /*PROGMEM*/ *toSend,
                               uint16_t sendn, int recn) {

  memcpy_P(sendbuffer, toSend, sendn);
  memset(recvbuffer, 0, recn);
  recvbuffer[recn] = 0xFF;

  asm volatile("call transaction_%0%1\n\t"
               :
               : "i"(P - 'A'), "i"(B)
               : "r18", "r19", "r20"

  );
  _delay_us(100);
}

#endif
