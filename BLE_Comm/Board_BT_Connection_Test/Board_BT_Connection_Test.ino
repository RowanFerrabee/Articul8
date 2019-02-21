// Select which type of test you'd like
// only one define supercedes following ones

#define LOOPBACK_TEST 0
#define INTERRUPT_TEST 1

#define TEST LOOPBACK_TEST

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

#define BATT_ALARM 3
void batteryAlarmInterrupt()
{
  // do something here :(  
}

void setupBatteryInterrupt()
{
  pinMode(BATT_ALARM, INPUT);
  delay(1);

  attachInterrupt(digitalPinToInterrupt(BATT_ALARM), batteryAlarmInterrupt, LOW);
}

void setup() {

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
