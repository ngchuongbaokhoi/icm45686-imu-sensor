// GyroSensor.h
#pragma once
#include "SensorBaseProvider.h"

class GyroSensor : public SensorBaseProvider {
public:
	GyroSensor();
	const char* channelType() const override { return "anglvel"; }
};