#ifndef _N64PAD_H
#define _N64PAD_H
#include <stdbool.h>
#include <stdint.h>

#define SENDBUFFER_SIZE (35 * 8 + 1)
#define RECVBUFFER_SIZE (33 * 8 + 1)

extern uint8_t recvbuffer[RECVBUFFER_SIZE];
extern uint8_t sendbuffer[SENDBUFFER_SIZE];

template <uint8_t P, uint8_t B> class N64Pad {

public:
  void init();
  void update();
  void setRumble(bool b);
  void initRumble();

private:
  void transaction(const uint8_t *toSend, uint16_t sendn, int recn);
};

#endif
