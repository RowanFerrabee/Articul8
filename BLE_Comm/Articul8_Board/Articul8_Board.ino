
#include "msg_defs.h"
#include "bt_man.h"
#include "imu_man.h"
#include "lra_man.h"
#include "fsm_man.h"

#define BT_RST 4
#define BT_BAUD 19200
#define LOGGER_BAUD 19200

Packet btCommand;
FSM_Man fsm_man = FSM_Man();

Packet ackPacket;

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

void setup() {

  // Initialize bluetooth pins through serial port
  pinMode(BT_RST, OUTPUT);
  digitalWrite(BT_RST, HIGH);
  BtSerial.begin(BT_BAUD);
  BtSerial.flush();
  Serial.begin(LOGGER_BAUD);
  Serial.flush();

  initAckPacket();
  initI2C();

  pinMode(INTERRUPT_PIN, INPUT);

  bool did_connect = false;
  mpu.initialize();
  did_connect = mpu.testConnection();
  if(!did_connect) { Serial.println("IMU testConnection failed"); while(1); }

  bool did_init = false;
  did_init = initDMP(0,0,0,0,0,0);
  if(!did_init) { Serial.println("DMP Init failed"); while(1); }

  dmpReady = true;
  
  Wire.begin(); // Initialize I2C
  initEnableMux(); // Enable all LRA drivers
  initLRAdrivers(); // Setup LRA drivers

  // Turn all motors off
  for (int i = 0; i < NUM_LRAS; i++) {
    lraIntensities[i] = 0;
    i2cMuxON(i); // Enable I2C for LRA driver 0 
    i2cWriteByte(DRV2604L_ADDR, 0x02, 0); // Zero power to LRA
    i2cMuxOFF(); // Again, when to do this?
  }
}

int toggle = 0;
unsigned loopCounter = 0;
bool command_available = false;

void loop() {

  loopCounter++;
  loopCounter %= 6;
  int l;
  
  switch(loopCounter)
  {
    case 0:
      command_available = checkForBTPacket((uchar *)btCommand.as_array);
      break;
      
    case 1:
      checkForIMUPacket(imuPacket.as_struct.data, &l);
      break;
      
    case 2:
      if (command_available)
      {
        switch (btCommand.as_struct.type)
        {
          case STANDBY:
            sendBTPacket(ackPacket.as_array);
            break;
          
          case STATE_CHANGE:
            fsm_man.changeStateFromMsg(btCommand.as_struct.data);
            sendBTPacket(ackPacket.as_array);
            break;
    
          case LRA_CONTROL:
            LRACmd lra_cmd((uchar *)btCommand.as_struct.data);
            executeLRACommand(lra_cmd);
            sendBTPacket(ackPacket.as_array);
            break;
        }
      }
      command_available = false;
      break;
      
    case 3:
      checkForIMUPacket(imuPacket.as_struct.data, &l);
      break;
      
    case 4:
      fsm_man.run_fsm();
      break;
      
    case 5:
      checkForIMUPacket(imuPacket.as_struct.data, &l);
      break;     
  }
}
