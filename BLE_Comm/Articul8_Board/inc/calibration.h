#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "msg_defs.h"

void calibrateDevice(uchar calibration_type);
uchar* reportOffsets();

#endif