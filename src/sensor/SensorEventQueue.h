#pragma once
#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <deque>
#include <mutex>

struct SensorVector {
	float   x;
	float   y;
	float   z;
	int8_t  status;
};

struct SensorEvent {
	int32_t      version;
	int32_t      sensor_id;
	int32_t      sensor_type;
	int64_t      timestamp_ns;
	SensorVector vector;
};

enum SensorType {
	ASENSOR_TYPE_ACCELEROMETER  = 1,
	ASENSOR_TYPE_MAGNETIC_FIELD = 2,
	ASENSOR_TYPE_GYROSCOPE      = 4,
};

class SensorEventQueue {
public:
	explicit SensorEventQueue(size_t capacity = 256) : _capacity(capacity) {}

	void enqueueEvent(const SensorEvent& event) {
		std::lock_guard<std::mutex> lk(_mutex);
		if (_queue.size() >= _capacity)
			_queue.pop_front();
		_queue.push_back(event);
	}

	ssize_t dequeueEvents(SensorEvent* out_events, size_t max_events) {
		if (!out_events) return -1;
		std::lock_guard<std::mutex> lk(_mutex);
		size_t n = 0;
		while (n < max_events && !_queue.empty()) {
			out_events[n++] = _queue.front();
			_queue.pop_front();
		}
		return (ssize_t)n;
	}

	size_t pendingCount() const {
		std::lock_guard<std::mutex> lk(_mutex);
		return _queue.size();
	}

private:
	mutable std::mutex      _mutex;
	std::deque<SensorEvent> _queue;
	size_t                  _capacity;
};
