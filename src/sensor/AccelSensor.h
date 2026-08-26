// AccelSensor.h
#pragma once
#include "SensorBaseProvider.h"

class AccelSensor : public SensorBaseProvider {
public:
	AccelSensor();
	const char* channelType() const override { return "accel"; }
};