#include "i2c_layer.h"


int8_t i2c_acquire(int8_t bus) { return 0; }
int8_t i2c_release(int8_t bus) { return 0; }

#ifdef USE_WIRE

//extern TwoWire Wire1;
void i2c_begin()
{
  Wire.begin();
//  Wire1.begin(); 
}

int8_t i2c_read_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *dst, uint8_t len, uint8_t iuno)
{
//    slaveAddress = slaveAddress >> 1;
    
    // 
    Wire.beginTransmission(slaveAddress);
    Wire.write(cmdCode);
    Wire.endTransmission();

    // 
    Wire.requestFrom(slaveAddress, len);

    int i = 0;
    while(Wire.available() && i < len)
    {
      dst[i++] = Wire.read();  
    }
        
    return 0;
}


#else

void i2c_begin()
{
  I2c.begin();
}

int8_t i2c_read_regs(uint8_t slaveAddress, uint8_t cmdCode, int16_t *value, uint8_t *crc)
{ 
  
    Serial.println("In read regs");
    if(!value || !crc)
      return -1;
    
    uint8_t status;

    Serial.println("Accessing ptrs");
    
    // assuming little endian, which I think is true in our case
    uint8_t *lb=(uint8_t*)value;
    uint8_t *hb=lb+1;

    Serial.println("Setting time out");
    
    I2c.timeOut(10);

    Serial.println("Sending slave address and cmd code :: calling i2c start");
    
    status = I2c.start();
    status = I2c.sendAddress(slaveAddress);
    status = I2c.sendByte(cmdCode);

    Serial.println("Reading slave address 3 bytes :: calling i2c start");
    status = I2c.start();
    status = I2c.sendAddress(slaveAddress | 0x01);
    
    status = I2c.receiveByte(1, lb);
    status = I2c.receiveByte(1, hb);
    status = I2c.receiveByte(0, crc);

    status = I2c.stop();

    return 0;
}

int8_t i2c_read_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *dst, uint8_t len, uint8_t iuno)
{ 
    (void)bus;
    (void)iuno;

    Serial.println("In read regs");
    
    if(!dst || len == 0)
      return -1;
    
    uint8_t status;

    // assuming little endian, which I think is true in our case

    I2c.timeOut(100);
    
    Serial.println("Starting transmission");
    status = I2c.start();
    status = I2c.sendAddress(slaveAddress | 0x01);
    status = I2c.sendByte(cmdCode);

    status = I2c.start();
    status = I2c.sendAddress(slaveAddress);

    int i;
    for(i = 0; i < len-1; ++i)
    {
      status = I2c.receiveByte(1, dst+i);  
    }
    
    status = I2c.receiveByte(0, dst+i);
    status = I2c.stop();

    return 0;
}


//    i2c_write_regs(dev->bus, dev->addr, crc_buf[1], send_buf, 3, 0);

int8_t i2c_write_regs(uint8_t bus, uint8_t slaveAddress, uint8_t cmdCode, uint8_t *src, uint8_t len, uint8_t iuno)
{ 
    (void)bus;
    (void)iuno;
    
    if(!src || len == 0)
      return -1;
    
    uint8_t status;

    // assuming little endian, which I think is true in our case
    
    status = I2c.start();
    status = I2c.sendAddress(slaveAddress | 0x01);
    status = I2c.sendByte(cmdCode);

    int i;
    for(i = 0; i < len-1; ++i)
    {
      status = I2c.receiveByte(1, src+i);  
    }
    
    status = I2c.receiveByte(0, src+i);
    status = I2c.stop();

    (void) status;

    return 0;
}

#endif
