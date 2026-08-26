#pragma once

#include <cstdint>
#include <string>

struct SensorEvent {
    int64_t timestamp_ns;
    float accel[3];	// m/s^2
    float gyro[3];	// rad/s
};

class SensorBaseProvider {
public:
    SensorBaseProvider();
    ~SensorBaseProvider();

    bool init();
    bool openDevice();
    bool start();
    bool stop();
    bool readSensor();
    bool convertSensorData();
    bool deinit();

    int poll(SensorEvent& event);

private:
    const int SAMPLE_RATE_HZ = 100;

    std::string _accel_sysfs; // /sys/bus/iio/devices/iio:deviceN
    std::string _gyro_sysfs;
    std::stirng _accel_devnode; // /dev/iio:deviceN
    std::string _gyro_devnode;

    int _accel_fd;
    int _gyro_fd;

    // Scale factors read from sysfs
    float _accel_scale;		// raw LSB -> m/s^2
    float _gyro_scale;		// raw LSB -> rad/s

    // Raw sample handed from readSensor() to convertSensorData()
    int32_t _raw_accel[3];
    int32_t _raw_gyro[3];
    int64_t _raw_timestamp_ns;

    // Converted result, published by poll()
    SensorEvent _last_event;

    // State
    bool _initialized;
    bool _streaming;
};
