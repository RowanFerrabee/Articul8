#ifndef SETUP_H
#define SETUP_H

#define BtSerial Serial1

#include "inc/msg_defs.h"
#include "inc/bt_man.h"

#define LOGGER_BAUD 9600

#define BT_RST_N 4
#define BT_SW_BTN 5
#define BT_WAKEUP 6
#define BT_BAUD 38400

extern Packet ackPacket;
extern Packet btCommand;

void initMPU();

void setupSerial()
{
  BtSerial.begin(BT_BAUD);
  BtSerial.flush();

  Serial.begin(LOGGER_BAUD);
  Serial.flush();
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

#endif
