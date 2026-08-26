#pragma once
#include "SensorBaseProvider.h"

class GyroSensor : public SensorBaseProvider {
public:
	GyroSensor();
	bool read(double& x, double& y, double& z) override;
private:
	double _scale = 0.0;
};
