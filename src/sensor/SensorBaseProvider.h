#pragma once

#include <cstdint>

struct SensorEvent {
    int64_t timestamp_ns;
    float accel[3];
    float gyro[3];
};

class SensorBaseProvider {
public:
    SensorBaseProvider();
    ~SensorBaseProvider();

    bool init();
    bool enable();
    bool disable();

    int poll(SensorEvent& event);

private:
    // TODO:
    // - IIO device/file descriptor
    // - sensor state
    // - configuration/state needed for Linux POSIX layer
};
