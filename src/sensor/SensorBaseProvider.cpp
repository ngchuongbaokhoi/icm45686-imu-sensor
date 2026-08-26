#include "SensorBaseProvider.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <vector>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#ifndef LINUX_PLATFORM
#error "requires -DLINUX_PLATFORM"
#endif

namespace fs = std::filesystem;
static const char* IIO_ROOT = "/sys/bus/iio/devices";

// Frame layout: s16 x, y, z + s16 pad + s64 timestamp = 16 bytes.
#pragma pack(push, 1)
struct IioFrame {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t pad;
	int64_t timestamp_ns;
};
#pragma pack(pop)
static_assert(sizeof(IioFrame) == 16, "frame must be 16 bytes");

SensorBaseProvider::SensorBaseProvider(std::string sensor_name, int32_t sensor_type)
	: _sensor_name(std::move(sensor_name)), _sensor_type(sensor_type) {}

SensorBaseProvider::~SensorBaseProvider() { deinit(); }

std::optional<std::string>
SensorBaseProvider::findDeviceByName(const std::string& name) const {
	std::error_code ec;
	if (!fs::exists(IIO_ROOT, ec)) return std::nullopt;
	for (const auto& dir : fs::directory_iterator(IIO_ROOT, ec)) {
		std::ifstream f(dir.path() / "name");
		std::string dn;
		if (f && std::getline(f, dn) && dn == name)
			return dir.path().string();
	}
	return std::nullopt;
}

int SensorBaseProvider::deviceIndex() const {
	auto pos = _dev_path.rfind("iio:device");
	if (pos == std::string::npos) return -1;
	return std::stoi(_dev_path.substr(pos + 10));
}

bool SensorBaseProvider::readSysfsDouble(const std::string& attr, double& out) const {
	std::ifstream f(_dev_path + "/" + attr);
	if (!f) return false;
	std::string s;
	if (!std::getline(f, s)) return false;
	try { out = std::stod(s); } catch (...) { return false; }
	return true;
}

bool SensorBaseProvider::writeSysfs(const std::string& attr, const std::string& val) const {
	std::string path = _dev_path + "/" + attr;
	int fd = ::open(path.c_str(), O_WRONLY);
	if (fd < 0) {
		std::cerr << "[sensor] open " << attr << ": " << std::strerror(errno) << "\n";
		return false;
	}
	ssize_t w = ::write(fd, val.c_str(), val.size());
	::close(fd);
	if (w < 0) {
		std::cerr << "[sensor] write " << attr << " (" << val << "): "
		          << std::strerror(errno) << "\n";
		return false;
	}
	return true;
}

bool SensorBaseProvider::init() {
	if (_ready) return true;
	auto found = findDeviceByName(_sensor_name);
	if (!found) {
		std::cerr << "[sensor] not found: " << _sensor_name << "\n";
		return false;
	}
	_dev_path = *found;
	_dev_index = deviceIndex();
	if (_dev_index < 0) { std::cerr << "[sensor] bad device index\n"; return false; }

	std::string scale_attr = std::string("in_") + channelType() + "_scale";
	if (!readSysfsDouble(scale_attr, _scale)) {
		std::cerr << "[sensor] no " << scale_attr << "\n";
		return false;
	}
	_ready = true;
	std::cerr << "[sensor] init: " << _sensor_name << " -> " << _dev_path
	          << " scale=" << _scale << "\n";
	return true;
}

void SensorBaseProvider::deinit() {
	stopSampling();
	_ready = false;
	_dev_path.clear();
}

bool SensorBaseProvider::enableBuffer(int hz) {
	const char* t = channelType();
	writeSysfs("buffer/enable", "0");                               // ensure off before config
	writeSysfs(std::string("scan_elements/in_") + t + "_x_en", "1");
	writeSysfs(std::string("scan_elements/in_") + t + "_y_en", "1");
	writeSysfs(std::string("scan_elements/in_") + t + "_z_en", "1");
	writeSysfs("scan_elements/in_temp_en", "0");                    // keep frame = xyz + ts
	writeSysfs("scan_elements/in_timestamp_en", "1");
	writeSysfs("sampling_frequency", std::to_string(hz));
	writeSysfs("buffer/length", "128");
	if (!writeSysfs("buffer/enable", "1")) {
		std::cerr << "[sensor] buffer/enable failed for " << _sensor_name << "\n";
		return false;
	}
	return true;
}

void SensorBaseProvider::disableBuffer() {
	writeSysfs("buffer/enable", "0");
}

bool SensorBaseProvider::startSampling(int32_t sampling_period_us) {
	if (!_ready || _running) return _running;
	int hz = sampling_period_us > 0 ? (int)(1e6 / sampling_period_us) : 100;

	if (!enableBuffer(hz)) return false;

	std::string dev = "/dev/iio:device" + std::to_string(_dev_index);
	_fd = ::open(dev.c_str(), O_RDONLY);          // blocking read
	if (_fd < 0) {
		std::cerr << "[sensor] open " << dev << ": " << std::strerror(errno) << "\n";
		disableBuffer();
		return false;
	}

	_running = true;
	_reader = std::thread(&SensorBaseProvider::bufferLoop, this);
	std::cerr << "[sensor] buffer sampling: " << _sensor_name << " @ " << hz << " Hz\n";
	return true;
}

void SensorBaseProvider::stopSampling() {
	if (!_running) return;
	_running = false;
	if (_fd >= 0) { ::close(_fd); _fd = -1; }     // close first so blocking read() returns
	if (_reader.joinable()) _reader.join();
	disableBuffer();
}

void SensorBaseProvider::bufferLoop() {
	const size_t FRAME = sizeof(IioFrame);
	const size_t BATCH = 32;
	std::vector<IioFrame> buf(BATCH);

	while (_running) {
		ssize_t n = ::read(_fd, buf.data(), BATCH * FRAME);   // blocking, read a batch
		if (n <= 0) {
			if (!_running) break;
			continue;
		}
		size_t cnt = (size_t)n / FRAME;
		for (size_t i = 0; i < cnt; i++) {
			SensorEvent ev;
			std::memset(&ev, 0, sizeof(ev));
			ev.version      = sizeof(SensorEvent);
			ev.sensor_type  = _sensor_type;
			ev.timestamp_ns = buf[i].timestamp_ns;
			ev.vector.x     = (float)(buf[i].x * _scale);
			ev.vector.y     = (float)(buf[i].y * _scale);
			ev.vector.z     = (float)(buf[i].z * _scale);
			_queue.enqueueEvent(ev);
		}
	}
}

ssize_t SensorBaseProvider::readEvents(SensorEvent* out_events, size_t max_events) {
	return _queue.dequeueEvents(out_events, max_events);
}