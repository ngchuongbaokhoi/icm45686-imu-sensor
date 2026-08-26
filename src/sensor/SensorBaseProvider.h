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

	bool startSampling(int32_t sampling_period_us);
	void stopSampling();

	ssize_t readEvents(SensorEvent* out_events, size_t max_events);

	const std::string& name() const { return _sensor_name; }
	int32_t type() const { return _sensor_type; }

	virtual const char* channelType() const = 0;   // "accel" | "anglvel"

protected:
	bool readSysfsDouble(const std::string& attr, double& out) const;
	bool writeSysfs(const std::string& attr, const std::string& val) const;

	std::string _type_str;

private:
	std::optional<std::string> findDeviceByName(const std::string& name) const;
	int  deviceIndex() const;
	bool enableBuffer(int hz);
	void disableBuffer();
	void bufferLoop();

	std::string       _sensor_name;
	std::string       _dev_path;
	int               _dev_index = -1;
	int32_t           _sensor_type;
	bool              _ready = false;
	double            _scale = 0.0;

	int               _fd = -1;
	SensorEventQueue  _queue;
	std::thread       _reader;
	std::atomic<bool> _running{false};
};