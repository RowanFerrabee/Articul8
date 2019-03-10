#ifndef SETUP_H
#define SETUP_H

#define BtSerial Serial1

#include "inc/msg_defs.h"
#include "inc/bt_man.h"
#include "Wire.h"
#define LOGGER_BAUD 9600

// bluetooth pins
#define BT_RST_N 4
#define BT_SW_BTN 5
#define BT_WAKEUP 6
#define BT_BAUD 38400

// fuel gauge pins
#define BATT_ALARM_PIN 3
#define FUEL_GAUGE_ADDRESS 0x16
#define ALARM_L_VOLT_REG 0x14

extern Packet ackPacket;
extern Packet btCommand;
extern lc709203f_t g_fuelGauge;
extern int battery_low_flag;

void initMPU();
void initFuelGauge();
void batteryAlarmCallback();

void setupSerial()
{
  BtSerial.begin(BT_BAUD);
  BtSerial.flush();

  Serial.begin(LOGGER_BAUD);
  Serial.flush();
}

void initI2C()
{
  Wire.begin();
}

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
  pinMode(BT_WAKEUP, INPUT); // leave high impedance

  setBtOn(true);
  delay(1);
  resetBT();
}

void initAckPacket()
{
  ackPacket.as_struct.sop = SOP;
  ackPacket.as_struct.type = ACK;
  
  for (int i = 0; i < PACKET_DATA_SIZE; i++)
  {
    ackPacket.as_struct.data[i] = '\0';
  }
  
  ackPacket.as_struct.checksum = SOP + ACK;
}

void initMPU()
{
  bool did_connect = false;
  mpu.initialize();
  did_connect = mpu.testConnection();
  if(!did_connect) { Serial.println("IMU testConnection failed"); while(1); }

  bool did_init = false;
  did_init = initDMP(0,0,0,0,0,0);
  if(!did_init) { Serial.println("DMP Init failed"); while(1); }

  dmpReady = true;  
}

void initFuelGauge()
{
  lc709203f_params_t params = {
    .alarm_pin = BATT_ALARM_PIN,
    .bus = 0,
    .addr = FUEL_GAUGE_ADDRESS >> 1
  };

  g_fuelGauge.cb = batteryAlarmCallback;
  lc709203f_init(&g_fuelGauge, &params);
  delayMicroseconds(200);
  lc709203f_set_alarm_low_cell_voltage(&g_fuelGauge, 3600);
  
}

void batteryAlarmCallback()
{
  battery_low_flag = 1;
}


#endif
