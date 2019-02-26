#ifndef I2C_LAYER_H
#define I2C_LAYER_H

#include <stdint.h>

#define USE_WIRE

#ifdef USE_WIRE
  #include "Wire.h"
#else
  #include "I2C.hpp"
#endif

void i2c_begin();
 
int8_t i2c_acquire(int8_t bus);
int8_t i2c_release(int8_t bus);

int8_t i2c_read_regs(uint8_t slaveAddress, uint8_t cmdCode, int16_t *value, uint8_t *crc);
int8_t i2c_read_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *dst, uint8_t len, uint8_t iuno);
int8_t i2c_write_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *src, uint8_t len, uint8_t iuno);

#endif
