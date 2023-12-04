#ifndef AHT21Sensor_h
#define AHT21Sensor_h

#include <Wire.h>

class AHT21Sensor {
public:
    AHT21Sensor();
    void begin();
    void update();
    float readTemperature() const;
    float readHumidity() const;

private:
    enum SensorState {
        INITIALIZING,
        START_MEASUREMENT,
        IDLE
    };

    float _temp;
    float _hum;
    uint8_t dataBuffer[6];
    unsigned long lastReadTime;
    unsigned long lastMeasurementTime;
    SensorState state;
    const uint8_t AHT21_ADDRESS = 0x38; // AHT21 I2C address

    void triggerMeasurement();
    void processMeasurement();
};

#endif
