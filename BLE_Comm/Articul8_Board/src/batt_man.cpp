
#include <string.h>
#include "../inc/batt_man.h"
#include "../inc/bt_man.h"

Packet batteryReport;

uchar* reportBatteryLevel(lc709203f_t* fuelGauge) {
	batteryReport.as_struct.type = BATTERY_REPORT;

	int16_t voltage = lc709203f_get_voltage(fuelGauge);
	memcpy(batteryReport.as_struct.data, &voltage, 2);

	populateBTPacket(batteryReport.as_array);
	return batteryReport.as_array;
}