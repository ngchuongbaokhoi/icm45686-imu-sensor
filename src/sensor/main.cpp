#include "SensorManager.h"
#include "AccelSensor.h"
#include "GyroSensor.h"
#include <memory>
#include <cstdio>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>

static void validateSampleRate(SensorManager& mgr, size_t idx, const char* label,
                               int duration_sec, double target_hz) {
	std::vector<int64_t> ts;
	auto t_end = std::chrono::steady_clock::now() + std::chrono::seconds(duration_sec);
	SensorEvent ev[128];
	while (std::chrono::steady_clock::now() < t_end) {
		ssize_t n = mgr.readEventsFrom(idx, ev, 128);
		for (ssize_t k = 0; k < n; k++)
			ts.push_back(ev[k].timestamp_ns);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	if (ts.size() < 3) {
		std::printf("  [%s] FAIL: too few samples (%zu)\n", label, ts.size());
		return;
	}

	double sum = 0, sum_sq = 0, dmin = 1e9, dmax = -1e9;
	size_t iv = ts.size() - 1;

	for (size_t i = 1; i < ts.size(); i++) {
		double d = (ts[i] - ts[i-1]) / 1e6;
		sum += d; sum_sq += d*d;
		if (d < dmin) dmin = d;
		if (d > dmax) dmax = d;
	}

	double mean = sum / iv;
	double jitter = std::sqrt(sum_sq / iv - mean*mean);
	double hz = 1000.0 / mean, tms = 1000.0 / target_hz;

	std::printf("  [%s] samples=%zu mean=%.3f ms (%.2f Hz) jitter=%.3f ms min=%.3f max=%.3f\n",
		label, ts.size(), mean, hz, jitter, dmin, dmax);

	bool rate_ok = std::fabs(hz - target_hz) <= target_hz * 0.02;
	bool jit_ok  = jitter < tms * 0.05;

	std::printf("  [%s] rate %s | jitter %s => %s\n",
		label, rate_ok?"OK":"OFF", jit_ok?"OK":"HIGH",
		(rate_ok && jit_ok) ? "PASS" : "FAIL");
}

int main() {
	SensorManager mgr;
	mgr.add(std::make_unique<AccelSensor>());
	mgr.add(std::make_unique<GyroSensor>());

	if (!mgr.initAll()) {
		std::printf("initAll failed\n");
		return 1;
	}

	mgr.startSamplingAll(10000);   // 100 Hz background

	SensorEvent ev[128];
	auto t_end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (std::chrono::steady_clock::now() < t_end) {
		for (size_t i = 0; i < mgr.count(); i++) {
			ssize_t n = mgr.readEventsFrom(i, ev, 128);
			if (n > 0) {
				const SensorEvent& e = ev[n-1];
				std::printf("sensor=%zu type=%d x=%.4f y=%.4f z=%.4f\n",
					i, e.sensor_type, e.vector.x, e.vector.y, e.vector.z);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	// Validate 100Hz sample rate with negligible jitter
	for (size_t i = 0; i < mgr.count(); i++)
		validateSampleRate(mgr, i, i == 0 ? "accel" : "gyro", 3, 100.0);

	mgr.stopSamplingAll();
	mgr.deinitAll();

	return 0;
}