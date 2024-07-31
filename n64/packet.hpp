#ifndef N64_PACKET_HPP_
#define N64_PACKET_HPP_

#include "packet.h"

#define ENCB(n, b, s) ((((~n) >> s) & 0x01) << b)
#define ENC(n, b)                                                              \
  ENCB(n, b, 7), ENCB(n, b, 6), ENCB(n, b, 5), ENCB(n, b, 4), ENCB(n, b, 3),   \
      ENCB(n, b, 2), ENCB(n, b, 1), ENCB(n, b, 0)
#define ENC8(n, b)                                                             \
  ENC(n, b), ENC(n, b), ENC(n, b), ENC(n, b), ENC(n, b), ENC(n, b), ENC(n, b), \
      ENC(n, b)
#define ENC32(n, b) ENC8(n, b), ENC8(n, b), ENC8(n, b), ENC8(n, b)

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::CONTROLLER_STATE[] = {ENC(1, B), 0xFF};

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::CONTROLLER_INFO[] = {ENC(0, B), 0xFF};

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::CONTROLLER_RESET[] = {ENC(0xFF, B), 0xFF};

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::RUMBLE_INIT[] = {
    ENC(0x03, B), ENC(0x80, B), ENC(0x01, B), ENC32(0x80, B), 0xFF};

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::RUMBLE_ON[] = {
    ENC(0x03, B), ENC(0xC0, B), ENC(0x1B, B), ENC32(0x01, B), 0xFF};

template <uint8_t B>
const uint8_t PROGMEM Packet<B>::RUMBLE_OFF[] = {
    ENC(0x03, B), ENC(0xC0, B), ENC(0x1B, B), ENC32(0x00, B), 0xFF};

template <uint8_t B> bool Packet<B>::pack(uint8_t *recvpacket, uint16_t n) {
  uint8_t d = 0;
  uint8_t b = 0x80;
  recvpacket[0] = 0;
  for (uint16_t s = 0; s < n; s++) {
    if (b == 0) { // needs to be here, at the end it would delete a new byte
      d++;
      recvpacket[d] = 0;
      b = 0x80;
    }
    uint8_t c = recvbuffer[s];
    if (c == 0xFF) {
      return false;
    }
    if (c == (1 << B)) {
      recvpacket[d] |= b;
    }
    b >>= 1;
  }
  return true;
}

#endif
