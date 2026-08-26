// GyroSensor.cpp
#include "GyroSensor.h"

GyroSensor::GyroSensor()
	: SensorBaseProvider("icm45686-gyro", ASENSOR_TYPE_GYROSCOPE) { _type_str = "anglvel"; }