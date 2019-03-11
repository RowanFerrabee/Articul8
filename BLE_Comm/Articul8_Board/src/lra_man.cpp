#include "../inc/lra_man.h"
#include "Arduino.h"
#include <Wire.h>

#define DRV2604L_ADDR (0x5A)

uint8_t lraIntensities[NUM_LRAS];

// Write a byte to corresponding register with given slave address
void lraWriteByte(uint8_t id, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(DRV2604L_ADDR ^ (id << 4));
  Wire.write(reg); // Register
  Wire.write(val); // Value
  Wire.endTransmission();
  delay(1);
}

void initLRAdrivers() {
  for (uint8_t i = 0; i < NUM_LRAS; i++) {
    lraWriteByte(i, 0x16, 0x50); // Set rated voltage (2.0V RMS at 205 Hz)
    lraWriteByte(i, 0x17, 0x92); // Set over-drive voltage (3.2V peak)
    lraWriteByte(i, 0x1A, 0xB6); // Set feedback control register to LRA type
    lraWriteByte(i, 0x20, 0x32); // Set to 205 Hz
    lraWriteByte(i, 0x1D, 0x81); // Set to open-loop
    lraWriteByte(i, 0x01, 0x05); // Set to RTP (real-time playback)
  }
}

LRACmd::LRACmd (PacketData data)
{
  if (data[0] == LRA_SPIN)
  {
    isSpinCmd = true;
    memcpy(&spinFreq, data+1, sizeof(float));
  }
  else
  {
    isSpinCmd = false;
    for (int i = 0; i < NUM_LRAS; i++) {
      intensities[i] = data[i+1];
    }
  }
}

void executeLRACommand(const LRACmd& lra_cmd)
{
  for (uint8_t id = 0; id < NUM_LRAS; id++)
    setLRAIntensity(id, lra_cmd.intensities[id]); 
}

int cachedIntensities[16] = {-1, -1, -1, -1,    -1, -1, -1, -1};

void setLRAIntensity(unsigned id, unsigned intensity)
{
  if(id < NUM_LRAS && intensity <= LRA_MAX_INTENSITY
   && intensity != cachedIntensities[id]
   )
  {
      lraWriteByte(id, 0x02, intensity); 
      cachedIntensities[id] = intensity;    
  }
}
