// AccelSensor.cpp
#include "AccelSensor.h"

AccelSensor::AccelSensor()
	: SensorBaseProvider("icm45686-accel", ASENSOR_TYPE_ACCELEROMETER) { _type_str = "accel"; }