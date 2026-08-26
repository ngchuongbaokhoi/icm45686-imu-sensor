#pragma once
#include "SensorBaseProvider.h"
#include <vector>
#include <memory>
#include <string>

class SensorManager {
public:
	SensorManager()  = default;
	~SensorManager() = default;

	void add(std::unique_ptr<SensorBaseProvider> sensor);

	bool initAll();
	void startSamplingAll(int32_t sampling_period_us);
	void stopSamplingAll();
	void deinitAll();

	ssize_t readEventsFrom(size_t idx, SensorEvent* out_events, size_t max_events);
	const std::string& nameOf(size_t idx) const;
	size_t count() const { return _sensors.size(); }

private:
	std::vector<std::unique_ptr<SensorBaseProvider>> _sensors;
};