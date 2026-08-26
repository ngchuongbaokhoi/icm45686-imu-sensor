#include "AccelSensor.h"

AccelSensor::AccelSensor()
	: SensorBaseProvider("icm45686-accel", ASENSOR_TYPE_ACCELEROMETER) {
		_type_str = "accel";
}

bool AccelSensor::read(double& x, double& y, double& z) {
	double rx, ry, rz;

	if (!readChannel("in_accel_x_raw", rx))
		return false;

	if (!readChannel("in_accel_y_raw", ry))
		return false;

	if (!readChannel("in_accel_z_raw", rz))
		return false;

	if (_scale == 0.0 && !readChannel("in_accel_scale", _scale))
		return false;

	x = rx * _scale;
	y = ry * _scale;
	z = rz * _scale;

	return true;
}
