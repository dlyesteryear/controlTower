#ifndef N64_PACKET_H_
#define N64_PACKET_H_

template <uint8_t B> struct Packet {
  static const uint8_t PROGMEM CONTROLLER_STATE[];
  static const uint8_t PROGMEM CONTROLLER_INFO[];
  static const uint8_t PROGMEM CONTROLLER_RESET[];
  static const uint8_t PROGMEM RUMBLE_INIT[];
  static const uint8_t PROGMEM RUMBLE_ON[];
  static const uint8_t PROGMEM RUMBLE_OFF[];
  static bool pack(uint8_t *dst, uint16_t);
};

#endif
