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
                               int dur_s, double target_hz) {
	std::vector<int64_t> ts;
	auto t_end = std::chrono::steady_clock::now() + std::chrono::seconds(dur_s);
	SensorEvent ev[256];
	while (std::chrono::steady_clock::now() < t_end) {
		ssize_t n = mgr.readEventsFrom(idx, ev, 256);
		for (ssize_t k = 0; k < n; k++) ts.push_back(ev[k].timestamp_ns);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	if (ts.size() < 3) { std::printf("  [%s] FAIL: few samples (%zu)\n", label, ts.size()); return; }
	double sum=0,sq=0,dmin=1e9,dmax=-1e9; size_t iv=ts.size()-1;
	for (size_t i=1;i<ts.size();i++){double d=(ts[i]-ts[i-1])/1e6;sum+=d;sq+=d*d;if(d<dmin)dmin=d;if(d>dmax)dmax=d;}
	double mean=sum/iv, jit=std::sqrt(sq/iv-mean*mean), hz=1000/mean, tms=1000/target_hz;
	std::printf("  [%s] samples=%zu mean=%.3fms (%.2fHz) jitter=%.3fms min=%.3f max=%.3f\n",
		label, ts.size(), mean, hz, jit, dmin, dmax);
	bool rok=std::fabs(hz-target_hz)<=target_hz*0.02, jok=jit<tms*0.05;
	std::printf("  [%s] rate %s | jitter %s => %s\n", label,
		rok?"OK":"OFF", jok?"OK":"HIGH", (rok&&jok)?"PASS":"FAIL");
}

int main() {
	SensorManager mgr;
	mgr.add(std::make_unique<AccelSensor>());
	mgr.add(std::make_unique<GyroSensor>());
	if (!mgr.initAll()) { std::printf("initAll failed\n"); return 1; }

	mgr.startSamplingAll(10000);   // 100 Hz via hardware buffer
	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	std::printf("=== STEP 1: read (5s) ===\n");
	SensorEvent ev[256];
	auto t_end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (std::chrono::steady_clock::now() < t_end) {
		for (size_t i=0;i<mgr.count();i++){
			ssize_t n = mgr.readEventsFrom(i, ev, 256);
			if (n>0){ const SensorEvent&e=ev[n-1];
				std::printf("%-16s x=%.4f y=%.4f z=%.4f\n",
					mgr.nameOf(i).c_str(), e.vector.x, e.vector.y, e.vector.z); }
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	std::printf("\n=== STEP 2: validate 100 Hz ===\n");
	for (size_t i=0;i<mgr.count();i++)
		validateSampleRate(mgr, i, mgr.nameOf(i).c_str(), 3, 100.0);

	mgr.stopSamplingAll();
	mgr.deinitAll();
	return 0;
}