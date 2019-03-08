
#include <string.h>
#include "../inc/calibration.h"
#include "../MPU/MPU6050.h"

Packet offsetReport;

void calibrateDevice(uchar calibration_type) {
	if (calibration_type == CALIBRATE_ACCEL)
	{
		int16_t xAccel, yAccel, zAccel;
		mpu.getAcceleration(&xAccel, &yAccel, &zAccel);

		mpu.setXAccelOffset(xAccel);
		mpu.setYAccelOffset(yAccel);
		mpu.setZAccelOffset(zAccel);
	}
	else if (calibration_type == CALIBRATE_GYRO)
	{
		int16_t xGyro, yGyro, zGyro;
		mpu.getRotation(&xGyro, &yGyro, &zGyro);

		mpu.setXGyroOffset(xGyro);
		mpu.setYGyroOffset(yGyro);
		mpu.setZGyroOffset(zGyro);
	}
}

uchar* reportOffsets() {

	offsetReport.as_struct.type = OFFSET_REPORT;

	int16_t xAccel = mpu.getXAccelOffset();
	int16_t yAccel = mpu.getYAccelOffset();
	int16_t zAccel = mpu.getZAccelOffset();
	int16_t xGyro = mpu.getXGyroOffset();
	int16_t yGyro = mpu.getYGyroOffset();
	int16_t zGyro = mpu.getZGyroOffset();

	memcpy(offsetReport.as_struct.data,    &xAccel, sizeof(xAccel));
	memcpy(offsetReport.as_struct.data+2,  &yAccel, sizeof(yAccel));
	memcpy(offsetReport.as_struct.data+4,  &zAccel, sizeof(zAccel));
	memcpy(offsetReport.as_struct.data+6,  &xGyro,  sizeof(xGyro));
	memcpy(offsetReport.as_struct.data+8,  &yGyro,  sizeof(yGyro));
	memcpy(offsetReport.as_struct.data+10, &zGyro,  sizeof(zGyro));

	populateBTPacket(offsetReport.as_array);
	return offsetReport.as_array;
}