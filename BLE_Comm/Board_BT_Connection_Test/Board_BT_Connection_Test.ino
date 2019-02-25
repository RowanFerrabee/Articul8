//#include "lc709203f_params.h"

// Select which type of test you'd like
// only one define supercedes following ones

#define LOOPBACK_TEST 0         // tests serial and bluetooth
#define FG_TEST 1               // tests i2c comm with the fuel gauge
#define FG_INTERRUPT_TEST 2     // tests the interrupt with the fuel gauge

#define TEST INTERRUPT_TEST

// ---------- actual code stuff ----------

#define BT_RST_N 4
#define BT_SW_BTN 5
#define BT_WAKEUP 6

#define BT_BAUD 19200

#define MOD_BAUD 19200

#define SERIAL_BUFFER_SIZE 100
#define NUM_TRANSMISSIONS 1000

char serialBuffer[SERIAL_BUFFER_SIZE];

#define BtSerial Serial1

void resetBT()
{
    pinMode(BT_RST_N, OUTPUT);
    delay(10);
    digitalWrite(BT_RST_N, LOW);
    delay(1);
    digitalWrite(BT_RST_N, HIGH);
    delay(1);
    pinMode(BT_RST_N, INPUT); // leave the reset pin in a high impedance state
}

void setBtOn(bool on) { digitalWrite(BT_SW_BTN, on ? HIGH : LOW); }
void setBtAwake(bool awake) { digitalWrite(BT_WAKEUP, awake ? LOW : HIGH); }

void setupBluetooth()
{
  pinMode(BT_RST_N, INPUT);  // leave the reset pin in a high impedance state
  pinMode(BT_SW_BTN, OUTPUT);
  pinMode(BT_WAKEUP, OUTPUT);

  delay(10);

  setBtOn(true);
  setBtAwake(true); // probably not relevant unless we put it in a sleeping mode  
}

#include "Wire.h"
#define BATT_ALARM 3
void batteryAlarmInterrupt()
{
  // do something here :(  
}

#define READ(addr) (addr | 0x01)
#define WRITE(addr) (addr)
#define FUEL_GAUGE ADDRESS 0x16
#define ALARM_L_VOLT_REG 0x14

void setupBatteryInterrupt()
{
  pinMode(BATT_ALARM, INPUT);
  delay(1);

  attachInterrupt(digitalPinToInterrupt(BATT_ALARM), batteryAlarmInterrupt, LOW);

  // tell the fuel gauge to alarm when battery voltage goes below (x)

  // not implemented :(
  
//  uint16_t mV = 3600;
//  uint8_t lb = mV & 0xFF;
//  uint8_t hb = (mV >> 8) & 0xFF;
  
//  uint8_t pwd = 0x87;
//
//  uint32_t msg = WRITE(FUEL_GAUGE_ADDRESS);
//  
//  msg = msg << 8;
//  msg |= ALARM_L_VOLT_REG;
//  msg = msg << 8;
//  msg |= lb;
//  msg = msg << 8l
//  msg |= hb;
//  
//  // compute crc 8 atm for the msg
//
//  
//  
//  Wire.beginTransmission(WRITE(FUEL_GAUGE_ADDRESS));
//  Wire.write(ALARM_L_VOLT_REG);
//  Wire.write(lb);
//  Wire.write(hb);
//
//  Wire.endTransmission(true);
}

void setup() {

    Wire.begin();
    setupBluetooth();

#if(TEST == INTERRUPT_TEST)
    setupBatteryInterrupt();
#endif

    // Initialize serial
    Serial.begin(MOD_BAUD);
    BtSerial.begin(BT_BAUD);

    Serial.flush();
    BtSerial.flush();


#if(TEST == INTERRUPT_TEST)

    setupBatteryInterrupt();
    
    
#endif

    delay(10);
}

#define MIN(x, y) ((x) > (y) ? (y) : (x))

void loopbackSerial(HardwareSerial &ser)
{
  int nb = ser.available();
  if (nb > 0) 
  {
    
    nb = MIN(nb, SERIAL_BUFFER_SIZE);
    ser.readBytes(serialBuffer, nb);
    
    Serial.write((unsigned char*)serialBuffer, nb);
    BtSerial.write((unsigned char*)serialBuffer, nb);
  }
  
}

void loop() {

#if(TEST == LOOPBACK_TEST)

  loopbackSerial(Serial);
  loopbackSerial(BtSerial);

#endif

}
