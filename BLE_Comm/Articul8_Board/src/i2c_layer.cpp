#include "../inc/i2c_layer.h"
#include "Wire.h"
#include "Arduino.h"

int8_t i2c_acquire(int8_t bus) { return 0; }
int8_t i2c_release(int8_t bus) { return 0; }

//extern TwoWire Wire1;
void i2c_begin()
{
    Wire.begin();
}

void i2c_scan()
{
    uint8_t error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for(address = 1; address < 127; address++ ) 
    {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address<16) 
                Serial.print("0");

            Serial.print(address,HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error==4) 
        {
            Serial.print("Unknown error at address 0x");
            if (address<16) 
                Serial.print("0");
            Serial.println(address,HEX);
        }    
    }

    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

}

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
