#ifndef LRA_MAN_H
#define LRA_MAN_H

#include <Wire.h>
#include "msg_defs.h"

#define TCA9554_ADDR (0x38)
#define TCA9548A_ADDR (0x70)
#define DRV2604L_ADDR (0x5A)

// Important: NUM_LRAS * 4 <= PACKET_DATA_SIZE
#define NUM_LRAS 8

extern uint8_t lraIntensities[];

// Write a byte to corresponding register with given slave address
void i2cWriteByte(uint8_t addr, uint8_t reg, uint8_t val);

// Sets enable pins on all drivers to HIGH (Uselss, will remove in new board design)
void initEnableMux();

// Turns MUX on for corresponding driver number
void i2cMuxON(unsigned int driver);

// Turns all channels off
void i2cMuxOFF();

void initLRAdrivers();

class LRACmd {
public:
  LRACmd (PacketData data);
  uint8_t intensities[NUM_LRAS];
};

void executeLRACommand(const LRACmd& lra_cmd);

#endif
