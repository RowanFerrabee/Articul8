#include <Wire.h>

#define LED_PIN_1 8
#define LED_PIN_2 9
#define TCA9554_ADDR (0x38)
#define TCA9548A_ADDR (0x70)
#define DRV2604L_ADDR (0x5A)

// Write a byte to corresponding register with given slave address
void i2cWriteByte(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg); // Register
  Wire.write(val); // Value
  Wire.endTransmission();
}

// Sets enable pins on all drivers to HIGH (Uselss, will remove in new board design)
void initEnableMux() {
  i2cWriteByte(TCA9554_ADDR, 0x03, 0); // Set enable_mux pins to output
  i2cWriteByte(TCA9554_ADDR, 0x01, 0xFF); // Set all pins to HIGH
}

// Turns MUX on for corresponding driver number
void i2cMuxON(unsigned int driver) {
  unsigned int i2c_mux = (1 << driver);
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(i2c_mux);
  Wire.endTransmission();
}

// Turns all channels off
void i2cMuxOFF() {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(0);
  Wire.endTransmission();
}

void initLRAdrivers() {
  for (int i = 0; i < 8; i++) {
    i2cMuxON(i); // Enable I2C for LRA driver i
    i2cWriteByte(DRV2604L_ADDR, 0x16, 0x46); // Set rated voltage (1.8V RMS at 235 Hz)
    i2cWriteByte(DRV2604L_ADDR, 0x17, 0x89); // Set over-drive voltage (3V peak)
    i2cWriteByte(DRV2604L_ADDR, 0x1A, 0xB6); // Set feedback control register to LRA type
    i2cWriteByte(DRV2604L_ADDR, 0x20, 0x2B); // Set to 235 Hz
    i2cWriteByte(DRV2604L_ADDR, 0x1D, 0x81); // Set to open-loop
    i2cWriteByte(DRV2604L_ADDR, 0x01, 0x05); // Set to RTP (real-time playback)
  }
}

void setup() {
  // Initialize LED pins
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);

  Wire.begin(); // Initialize I2C
  initEnableMux(); // Enable all LRA drivers
  initLRAdrivers(); // Setup LRA drivers
  
  i2cMuxON(0); // Enable I2C for LRA driver 0
  i2cWriteByte(DRV2604L_ADDR, 0x02, 127); // Run at max power for 3 seconds
  delay(3000);
  i2cWriteByte(DRV2604L_ADDR, 0x02, 0);

  i2cMuxON(6); // Enable I2C for LRA driver 6
  i2cWriteByte(DRV2604L_ADDR, 0x02, 127); // Run at max power for 3 seconds
  delay(3000);
  i2cWriteByte(DRV2604L_ADDR, 0x02, 0);

  // Run two motors simultaneously
  i2cMuxON(0); // Enable I2C for LRA driver 0
  i2cWriteByte(DRV2604L_ADDR, 0x02, 127);
  delay(1500);

  i2cMuxON(6); // Enable I2C for LRA driver 6
  i2cWriteByte(DRV2604L_ADDR, 0x02, 127);
  delay(1500);

  i2cMuxON(0); // Enable I2C for LRA driver 0
  i2cWriteByte(DRV2604L_ADDR, 0x01, 0x40); // End RTP playback
  delay(1500);

  i2cMuxON(6); // Enable I2C for LRA driver 6
  i2cWriteByte(DRV2604L_ADDR, 0x01, 0x40); // End RTP playback
  
  i2cMuxOFF(); // Disconnect I2C mux from all channels
}

void loop() {}
