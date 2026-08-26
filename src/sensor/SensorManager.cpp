#include "SensorManager.h"

void SensorManager::add(std::unique_ptr<SensorBaseProvider> sensor) {
	_sensors.push_back(std::move(sensor));
}

bool SensorManager::initAll() {
	bool ok = true;
	for (auto& s : _sensors)
		if (!s->init()) ok = false;
	return ok;
}

void SensorManager::startSamplingAll(int32_t sampling_period_us) {
	for (auto& s : _sensors) s->startSampling(sampling_period_us);
}

void SensorManager::stopSamplingAll() {
	for (auto& s : _sensors) s->stopSampling();
}

void SensorManager::deinitAll() {
	for (auto& s : _sensors) s->deinit();
}

ssize_t SensorManager::readEventsFrom(size_t idx, SensorEvent* out_events, size_t max_events) {
	if (idx >= _sensors.size()) return -1;
	return _sensors[idx]->readEvents(out_events, max_events);
}

const std::string& SensorManager::nameOf(size_t idx) const {
	static const std::string empty = "unknown";
	if (idx >= _sensors.size()) return empty;
	return _sensors[idx]->name();
}