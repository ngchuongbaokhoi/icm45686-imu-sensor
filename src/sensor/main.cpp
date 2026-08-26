#include "SensorManager.h"
#include "AccelSensor.h"
#include "GyroSensor.h"
#include <memory>
#include <cstdio>
#include <thread>
#include <chrono>

int main() {
	SensorManager mgr;
	mgr.add(std::make_unique<AccelSensor>());
	mgr.add(std::make_unique<GyroSensor>());

	if (!mgr.initAll()) {
		std::printf("initAll failed\n");
		return 1;
	}

	mgr.startSamplingAll(10000);

	auto t_end = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	SensorEvent events[128];
	while (std::chrono::steady_clock::now() < t_end) {
		for (size_t i = 0; i < mgr.count(); i++) {
			ssize_t n = mgr.readEventsFrom(i, events, 128);
			if (n < 0) continue;
			for (ssize_t k = 0; k < n; k++)
				std::printf("sensor=%zu type=%d ts=%ld x=%.4f y=%.4f z=%.4f\n",
					i, events[k].sensor_type, (long)events[k].timestamp_ns,
					events[k].vector.x, events[k].vector.y, events[k].vector.z);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	mgr.stopSamplingAll();
	mgr.deinitAll();

	return 0;
}
