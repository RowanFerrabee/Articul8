#ifndef ARTICUL8_FSM_MAN_H
#define ARTICUL8_FSM_MAN_H

#include "msg_defs.h"
#include "imu_man.h"
#include <Arduino.h>
#include "lra_rotation.h"

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

  float spinFreq = 0;

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
    lra_rotate_setup();
    lra_rotate_setIntensity(LRA_MAX_INTENSITY);
  }

  void setSpinFreq(float spinFreq) {
    this->spinFreq = spinFreq;
    lra_rotate_setFrequency(spinFreq);
  }

  void changeState(uchar state, int period) {
    this->state = static_cast<BoardState>(state);
    this->period = period;

    // state specific transition actions could be handled more cleanly
    if(this->state == IMU_STREAMING_STATE) { timeout = DEFAULT_IMU_TIMEOUT; }
  }

  void changeStateFromMsg(uchar *msg) {
    uchar state = msg[0];
    int period;
    memcpy(&period, &msg[1], sizeof(int));
    changeState(state, period);
  }

  void lra_fsm(int ms)
  {
    if (spinFreq != 0) {
      lra_rotate_count(ms);
      int intensities[NUM_LRAS*2] = {0, 0, 0, 0, 0, 0, 0, 0};
      lra_rotate_getOutputs(intensities);

      for(int i = 0; i < NUM_LRAS; ++i)
      {
        // setLRAIntensity(i, 0);
        setLRAIntensity(i, intensities[i]);
      }
    }
  }

  void imu_fsm()
  {
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
        Serial.print("FSM in invalid state: ");
        Serial.println(state);
        break;
    }    
  }

  void run_fsm() {

    // only run the FSM if the period has been reached
    int currentMs = millis();
    if(currentMs - lastCycleStart > period) { lastCycleStart += period; }
    else { return; }

    imu_fsm();    
    lra_fsm(period);
  }
};

#endif
