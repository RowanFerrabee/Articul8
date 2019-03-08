#ifndef LRA_MAN_H
#define LRA_MAN_H

#include "msg_defs.h"
#include "stdint.h"

#define NUM_LRAS 8
#define LRA_MAX_INTENSITY 127
#define LRA_MIN_INTENSITY 0

// Write a byte to corresponding LRA
void lraWriteByte(uint8_t id, uint8_t reg, uint8_t val);
void initLRAdrivers();

struct LRACmd {
  LRACmd (PacketData data);
  uint8_t intensities[NUM_LRAS];
};

void executeLRACommand(const LRACmd& lra_cmd);
void setLRAIntensity(unsigned id, unsigned intensity);

#endif
