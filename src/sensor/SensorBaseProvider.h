#pragma once
#include "SensorEventQueue.h"
#include <string>
#include <optional>
#include <thread>
#include <atomic>

class SensorBaseProvider {
public:
	SensorBaseProvider(std::string sensor_name, int32_t sensor_type);
	virtual ~SensorBaseProvider();

	bool init();
	void deinit();
	bool isReady() const { return _ready; }

	bool startSampling(int32_t period_us);
	void stopSampling();

	ssize_t readEvents(SensorEvent* out_events, size_t max_events);

	const std::string& name() const { return _sensor_name; }
	int32_t type() const { return _sensor_type; }

	virtual bool read(double& x, double& y, double& z) = 0;

protected:
	bool readChannel(const std::string& attr, double& out) const;
	std::string _type_str;

private:
	std::optional<std::string> findDeviceByName(const std::string& name) const;
	void samplingLoop(int32_t period_us);

	std::string       _sensor_name;
	std::string       _dev_path;
	int32_t           _sensor_type;
	bool              _ready = false;

	SensorEventQueue  _queue;
	std::thread       _sampler;
	std::atomic<bool> _sampling{false};
};
