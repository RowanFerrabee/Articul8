#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "msg_defs.h"
#include "bt_man.h"
#include "imu_man.h"

void calibrateDevice(uchar calibration_type);
uchar* reportOffsets();

#endif