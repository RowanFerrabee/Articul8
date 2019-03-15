
#define BtSerial Serial1

#include "inc/msg_defs.h"
#include "inc/batt_man.h"
#include "inc/bt_man.h"
#include "inc/calibration.h"
#include "inc/imu_man.h"
#include "inc/lra_man.h"
#include "inc/fsm_man.h"
#include "inc/lc709203f.h"
#include "articul8Setup.h"

Packet ackPacket;
Packet btCommand;
lc709203f_t g_fuelGauge;
FSM_Man fsm_man = FSM_Man();

int battery_low_flag = 0;
int toggle = 0;
unsigned loopCounter = 0;
bool command_available = false;

void loop() {

  if (battery_low_flag) {
    delay(500);
    return;
  }

  loopCounter++;
  loopCounter %= 4;
  int l;
  
  switch(loopCounter)
  {
    case 0:
      command_available = checkForBTPacket((uchar *)btCommand.as_array);
      break;
      
    case 1:
      checkForIMUPacket(imuPacket.as_struct.data, &l);
      fsm_man.run_fsm();
      break;
      
    case 2:
      if (command_available)
      {
        switch (btCommand.as_struct.type)
        {
          case STANDBY:
            sendBTPacket(ackPacket.as_array);
            break;

          case CALIBRATE:
            calibrateDevice(btCommand.as_struct.data[0]);
            sendBTPacket(reportOffsets());
            break;

          case OFFSET_REPORT:
            sendBTPacket(reportOffsets());
            break;

          case BATTERY_REPORT:
            sendBTPacket(reportBatteryLevel(&g_fuelGauge));
            break;
          
          case STATE_CHANGE:
            fsm_man.changeStateFromMsg(btCommand.as_struct.data);
            sendBTPacket(ackPacket.as_array);
            break;
    
          case LRA_CONTROL:
            LRACmd lra_cmd((uchar *)btCommand.as_struct.data);
            if (lra_cmd.isSpinCmd)
            {
              fsm_man.setSpinFreq(lra_cmd.spinFreq);
              fsm_man.setSpintensity(lra_cmd.spintensity);
            }
            else
            {
              fsm_man.setSpinFreq(0);
              executeLRACommand(lra_cmd);
            }
            sendBTPacket(ackPacket.as_array);
            break;
        }
      }
      
      command_available = false;
      break;
      
    case 3:
      checkForIMUPacket(imuPacket.as_struct.data, &l);
      fsm_man.run_fsm();
      break;     
  }

}

void setup() {

  // Initialize bluetooth pins through serial port
  setupBluetooth();
  setupSerial();
  initI2C();
  initAckPacket();
  initMPU();
  initFuelGauge();
  initLRAdrivers();
  
}
