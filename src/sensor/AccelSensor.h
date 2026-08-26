#pragma once
#include "SensorBaseProvider.h"

class AccelSensor : public SensorBaseProvider {
public:
	AccelSensor();
	bool read(double& x, double& y, double& z) override;
private:
	double _scale = 0.0;
};
