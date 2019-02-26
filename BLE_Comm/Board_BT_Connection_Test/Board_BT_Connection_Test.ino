#include "lc709203f.h"

#include "i2c_layer.h"

// Select which type of test you'd like
// only one define supercedes following ones

#define LOOPBACK_TEST 0         // tests serial and bluetooth
#define FG_TEST 1               // tests i2c comm with the fuel gauge
#define FG_INTERRUPT_TEST 2     // tests the interrupt with the fuel gauge

#define TEST FG_INTERRUPT_TEST

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

#define BATT_ALARM_PIN 3
void batteryAlarmInterrupt()
{
  // do something here :(  
}

#define READ(addr) (addr | 0x01)
#define WRITE(addr) (addr)
#define FUEL_GAUGE_ADDRESS 0x16
#define ALARM_L_VOLT_REG 0x14

lc709203f_t g_fuelGauge;

void setupFuelGauge()
{
  lc709203f_params_t params = {
    .alarm_pin = BATT_ALARM_PIN,
    .bus = 0,
    .addr = FUEL_GAUGE_ADDRESS
  };
  
  lc709203f_init(&g_fuelGauge, &params);
}

void setup() {

  i2c_begin();

#if(TEST == LOOPBACK_TEST)    
    setupBluetooth();
#endif

#if(TEST == FG_INTERRUPT_TEST)
  setupFuelGauge();
#endif
    
    // Initialize serial
    Serial.begin(MOD_BAUD);
    BtSerial.begin(BT_BAUD);

    Serial.flush();
    BtSerial.flush();
    
    I2c.scan();
    while(1){}

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

#elif(TEST == FG_INTERRUPT_TEST)
  Serial.println("Reading...");
  int16_t mV = lc709203f_get_voltage(&g_fuelGauge);
  Serial.println(mV);

  delay(1000);

#endif

}
