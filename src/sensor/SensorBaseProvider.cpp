#include "SensorBaseProvider.h"

// TODO:
// Implement Linux/POSIX sensor provider.
//
// Responsibilities:
// 1. Open/configure IIO device
// 2. Enable/disable sensor
// 3. Read IIO samples
// 4. Convert IIO data to SensorEvent
// 5. Expose polling interface for SensorManager.cpp
// 6. Keep implementation Linux-specific

SensorBaseProvider::SensorBaseProvider()
{
    // TODO
}

SensorBaseProvider::~SensorBaseProvider()
{
    // TODO
}

bool SensorBaseProvider::init()
{
    // TODO
    return false;
}

bool SensorBaseProvider::enable()
{
    // TODO
    return false;
}

bool SensorBaseProvider::disable()
{
    // TODO
    return false;
}

int SensorBaseProvider::poll(SensorEvent& event)
{
    // TODO
    return -1;
}
