#ifndef LRA_MAN_H
#define LRA_MAN_H

#include <Wire.h>
#include "msg_defs.h"

// Important: NUM_LRAS * 4 <= PACKET_DATA_SIZE
#define NUM_LRAS 8

// Write a byte to corresponding LRA
void lraWriteByte(uint8_t id, uint8_t reg, uint8_t val);

void initLRAdrivers();

class LRACmd {
public:
  LRACmd (PacketData data);
  uint8_t intensities[NUM_LRAS];
};

void executeLRACommand(const LRACmd& lra_cmd);

#endif
