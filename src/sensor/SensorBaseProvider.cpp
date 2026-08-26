#include "SensorBaseProvider.h"

const char* IIO_DEVICES_DIR = "/sys/buss/iio/devices";

SensorBaseProvider::SensorBaseProvider()
	:_accel_fd(-1),
	 _gyro_fd(-1),
	 _accel_scale(0.0f),
	 _gyro_scale(0.0f),
	 _initialized(false),
	 _streaming(false)

{

}

SensorBaseProvider::~SensorBaseProvider()
{
    deinit();
}

bool SensorBaseProvider::init()
{
    if (_initialized) {
        return true;
    }

    if (!findIIODeviceByName("icm45686-accel", _accel_sysfs, _accel_devnode)) {
        fprintf(stderr, "init: accel IIO device not found!\n");
	return false;
    }

    if (!findIIODeviceByName("icm45686-gyro", _gyro_sysfs, _gyro_devnode)) {
        fprintf(stderr, "init: gyro IIO device not found!\n");
	return false;
    }

    _initialized = true;
    return true;
}

bool SensorBaseProvider::openDevice()
{
    // TODO
    return false;
}

bool SensorBaseProvider::start()
{
    // TODO
    return false;
}

bool SensorBaseProvider::stop()
{
    // TODO
    return false;
}

bool SensorBaseProvider::readSensor()
{
    // TODO
    return false;
}

bool SensorBaseProvider::convertSensorData()
{
    // TODO
    return false;
}

int SensorBaseProvider::poll(SensorEvent& event)
{
    // TODO
    return -1;
}

bool SensorBaseProvider::deinit()
{
    // TODO
    return false;
}
