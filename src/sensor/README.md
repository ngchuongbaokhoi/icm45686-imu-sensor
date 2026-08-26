# IMU ICM45686 sensor middleware

## Files
- SensorEventQueue.h   : the FIFO event queue (single interface, no other FIFO)
- SensorBaseProvider.* : generic init / deinit / read + background sampler
- AccelSensor.*        : accelerometer
- GyroSensor.*         : gyroscope
- SensorManager.*      : create -> init -> read 10s -> deinit
- main.cpp             : entry point
- Makefile             : cross-compile for RUBIK Pi 3

## Build
    cd src/sensor
    make                       # normal
    make CXXFLAGS="-std=c++17 -O2 -pthread -DLINUX_PLATFORM -DDEBUG"   # with debug logs

## Run
    scp imu_app ubuntu@<board-ip>:/var/tmp/
    sudo /var/tmp/imu_app
