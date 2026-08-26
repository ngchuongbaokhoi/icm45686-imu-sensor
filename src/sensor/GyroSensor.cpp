#include "GyroSensor.h"

GyroSensor::GyroSensor()
	: SensorBaseProvider("icm45686-gyro", ASENSOR_TYPE_GYROSCOPE) { 
		_type_str = "anglvel";
}

bool GyroSensor::read(double& x, double& y, double& z) {
	double rx, ry, rz;

	if (!readChannel("in_anglvel_x_raw", rx))
		return false;

	if (!readChannel("in_anglvel_y_raw", ry))
		return false;

	if (!readChannel("in_anglvel_z_raw", rz))
		return false;

	if (_scale == 0.0 && !readChannel("in_anglvel_scale", _scale))
		return false;

	x = rx * _scale;
	y = ry * _scale;
	z = rz * _scale;

	return true;
}
