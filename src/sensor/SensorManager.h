#pragma once
#include "SensorBaseProvider.h"
#include <vector>
#include <memory>

class SensorManager {
public:
	SensorManager()  = default;
	~SensorManager() = default;

	void add(std::unique_ptr<SensorBaseProvider> sensor);

	bool initAll();
	void startSamplingAll(int32_t period_us);
	void stopSamplingAll();
	void deinitAll();

	ssize_t readEventsFrom(size_t idx, SensorEvent* out_events, size_t max_events);
	size_t count() const { return _sensors.size(); }

private:
	std::vector<std::unique_ptr<SensorBaseProvider>> _sensors;
};
