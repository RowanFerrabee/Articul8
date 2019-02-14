#ifndef ARTICUL8_FSM_MAN_H
#define ARTICUL8_FSM_MAN_H

#include "msg_defs.h"
#include "logging.h"
#include "imu_man.h"
#include <Arduino.h>

#define DEFAULT_PERIOD 1000
#define DEFAULT_IMU_TIMEOUT 2000

enum BoardState {
  DEFAULT_STATE = 0,
  IMU_STREAMING_STATE = 1,
  INVALID_STATE = 2,
  NUM_BOARD_STATES
};

class FSM_Man {
  
private:
  BoardState state = DEFAULT_STATE;
  int period = 1000;
  int lastCycleStart;

  Packet standbyPacket;
  int timeout = 0;
  bool newImuPacket = false;
  
public:
  FSM_Man() 
  {
    this->lastCycleStart = millis();
    
    standbyPacket.as_struct.sop = SOP;
    standbyPacket.as_struct.type = STANDBY;
    for (int i = 0; i < PACKET_DATA_SIZE; i++)
    {
      standbyPacket.as_struct.data[i] = '\0';
    }
    
    standbyPacket.as_struct.checksum = SOP + ACK;
  }

  void changeState(uchar state, int period) {
    this->state = static_cast<BoardState>(state);
    this->period = period;
    this->lastCycleStart = millis();

    // state specific transition actions could be handled more cleanly
    if(this->state == IMU_STREAMING_STATE) { timeout = DEFAULT_IMU_TIMEOUT; }
  }

  void changeStateFromMsg(uchar *msg) {
    uchar state = msg[0];
    int period;
    memcpy(&period, &msg[1], sizeof(int));
    changeState(state, period);
  }

  void run_fsm() {

    // only run the FSM if the period has been reached
    int currentMs = millis();
    if(currentMs - lastCycleStart > period) { lastCycleStart += period; }
    else { return; }
    
    switch (state) {
      case DEFAULT_STATE:
//        sendBTPacket(standbyPacket.as_array);
        break;

      case IMU_STREAMING_STATE:
        imuPacket.as_struct.type = IMU_DATA;
        populateBTPacket(imuPacket.as_array);
        sendBTPacket(imuPacket.as_array);
        
        timeout -= period;
        if(timeout < 1) { timeout = 0; changeState(DEFAULT_STATE, DEFAULT_PERIOD); }
        
        break;

      default:
        articul8Logger.print("FSM in invalid state: ");
        articul8Logger.println(state);
        break;
    }
  }
};

#endif
