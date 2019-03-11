#ifndef BATT_MAN_H
#define BATT_MAN_H

#include "msg_defs.h"
#include "lc709203f.h"

uchar* reportBatteryLevel(lc709203f_t* fuelGauge);

#endif