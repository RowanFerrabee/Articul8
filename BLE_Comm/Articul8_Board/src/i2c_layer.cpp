#include "../inc/i2c_layer.h"
#include "Wire.h"


int8_t i2c_acquire(int8_t bus) { return 0; }
int8_t i2c_release(int8_t bus) { return 0; }

//extern TwoWire Wire1;
void i2c_begin()
{
  Wire.begin();
}

// void i2c_scan()
// {
// //  Serial.println("I2C scan not implemented with wire...");
// }

int8_t i2c_read_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *dst, uint8_t len, uint8_t iuno)
{
    slaveAddress = slaveAddress;
    
    // 
    Wire.beginTransmission(slaveAddress);
    Wire.write(cmdCode);
    Wire.endTransmission(false);

    // 
    Wire.requestFrom(slaveAddress, len);

    int i = 0;
    while(Wire.available() && i < len)
    {
      dst[i++] = Wire.read();  
    }
        
    return 0;
}

int8_t i2c_write_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *src, uint8_t len, uint8_t iuno)
{ 
    (void)bus;
    (void)iuno;
    
    if(!src || len == 0)
      return -1;
    
    Wire.beginTransmission(slaveAddress);
    Wire.write(cmdCode);

    int i;
    for(i = 0; i < len-1; ++i)
    {
      Wire.write(src[i]);  
    }
    
    Wire.endTransmission();

    return 0;
}
