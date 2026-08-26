#include "SensorBaseProvider.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include <pthread.h>

namespace fs = std::filesystem;
static const char* IIO_ROOT = "/sys/bus/iio/devices";
static std::mutex g_bus_mutex;

static inline int64_t now_ns() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

SensorBaseProvider::SensorBaseProvider(std::string sensor_name, int32_t sensor_type)
	: _sensor_name(std::move(sensor_name)), _sensor_type(sensor_type) {}

SensorBaseProvider::~SensorBaseProvider() {
	deinit();
}

std::optional<std::string>
SensorBaseProvider::findDeviceByName(const std::string& name) const {
	std::error_code ec;
	if (!fs::exists(IIO_ROOT, ec))
		return std::nullopt;

	for (const auto& dir : fs::directory_iterator(IIO_ROOT, ec)) {
		std::ifstream f(dir.path() / "name");
		std::string dev_name;
		if (f && std::getline(f, dev_name) && dev_name == name)
			return dir.path().string();
	}

	return std::nullopt;
}

bool SensorBaseProvider::init() {
	if (_ready)
		return true;

	auto found = findDeviceByName(_sensor_name);

	if (!found) {
#ifdef DEBUG
		std::cerr << "[DBG] not found: " << _sensor_name << "\n";
#endif
		return false;
	}

	_dev_path = *found;
	_ready = true;
#ifdef DEBUG
	std::cerr << "[DBG] init: " << _sensor_name << " -> " << _dev_path << "\n";
#endif

	return true;
}

void SensorBaseProvider::deinit() {
	stopSampling();
#ifdef DEBUG
	if (_ready)
		std::cerr << "[DBG] deinit: " << _sensor_name << "\n";
#endif
	_ready = false;
	_dev_path.clear();
}

bool SensorBaseProvider::readChannel(const std::string& attr, double& out) const {
	if (!_ready)
		return false;

	std::lock_guard<std::mutex> bus(g_bus_mutex);
	std::ifstream f(_dev_path + "/" + attr);

	if (!f)
		return false;

	std::string s;

	if (!std::getline(f, s))
		return false;

	try { out = std::stod(s); } catch (...) { return false; }

	return true;
}

bool SensorBaseProvider::startSampling(int32_t period_us) {
	if (!_ready)
		return false;
	
	if (_sampling)
		return true;

	if (period_us <= 0)
		period_us = 10000;

	_sampling = true;
	_sampler = std::thread(&SensorBaseProvider::samplingLoop, this, period_us);

	return true;
}

void SensorBaseProvider::stopSampling() {
	if (!_sampling)
		return;

	_sampling = false;

	if (_sampler.joinable())
		_sampler.join();
}

void SensorBaseProvider::samplingLoop(int32_t period_us) {
	struct timespec next;
	clock_gettime(CLOCK_MONOTONIC, &next);
	const int64_t period_ns = (int64_t)period_us * 1000;

	while (_sampling) {
		double x, y, z;
		if (read(x, y, z)) {
			SensorEvent ev;
			std::memset(&ev, 0, sizeof(ev));
			ev.version      = sizeof(SensorEvent);
			ev.sensor_type  = _sensor_type;
			ev.timestamp_ns = now_ns();
			ev.vector.x     = (float)x;
			ev.vector.y     = (float)y;
			ev.vector.z     = (float)z;
			_queue.enqueueEvent(ev);
#ifdef DEBUG
			std::cerr << "[DBG] " << _sensor_name << " sample x=" << x
				<< " y=" << y << " z=" << z << "\n";
#endif
		}

		next.tv_nsec += period_ns;
		while (next.tv_nsec >= 1000000000LL) { next.tv_nsec -= 1000000000LL; next.tv_sec++; }

		struct timespec now_ts;

		clock_gettime(CLOCK_MONOTONIC, &now_ts);
		if (now_ts.tv_sec > next.tv_sec ||
		    (now_ts.tv_sec == next.tv_sec && now_ts.tv_nsec > next.tv_nsec))
			next = now_ts;
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
	}
}

ssize_t SensorBaseProvider::readEvents(SensorEvent* out_events, size_t max_events) {
	return _queue.dequeueEvents(out_events, max_events);
}
