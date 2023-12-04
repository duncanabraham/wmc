#include "AHT21Sensor.h"

AHT21Sensor::AHT21Sensor() : _temp(0.0), _hum(0.0), lastReadTime(0), lastMeasurementTime(0), state(IDLE) {
    // Constructor
}

void AHT21Sensor::begin() {
    Wire.begin();
    Wire.beginTransmission(AHT21_ADDRESS);
    Wire.write(0xBE); // Initialization command for AHT21
    Wire.write(0x08);
    Wire.write(0x00);
    Wire.endTransmission();
    lastReadTime = millis();
    state = INITIALIZING;
}

void AHT21Sensor::update() {
    unsigned long currentTime = millis();

    // Trigger new measurement every second
    if (currentTime - lastMeasurementTime > 1000 && state == IDLE) {
        triggerMeasurement();
        lastMeasurementTime = currentTime;
    }

    switch (state) {
        case INITIALIZING:
            if (currentTime - lastReadTime > 10) { // Wait for sensor to initialize
                state = IDLE;
            }
            break;

        case START_MEASUREMENT:
            if (currentTime - lastReadTime > 80) { // Wait for measurement
                Wire.requestFrom(AHT21_ADDRESS, 6);
                if (Wire.available() == 6) {
                    for (int i = 0; i < 6; i++) {
                        dataBuffer[i] = Wire.read();
                    }
                    processMeasurement();
                }
                state = IDLE;
            }
            break;

        case IDLE:
            // Ready for next measurement
            break;
    }
}

void AHT21Sensor::triggerMeasurement() {
    if (state == IDLE) {
        Wire.beginTransmission(AHT21_ADDRESS);
        Wire.write(0xAC); // Command to trigger measurement
        Wire.write(0x33);
        Wire.write(0x00);
        Wire.endTransmission();
        lastReadTime = millis();
        state = START_MEASUREMENT;
    }
}

void AHT21Sensor::processMeasurement() {
    // Process temperature
    unsigned int temp = ((unsigned int)dataBuffer[3] & 0x0F) << 16 | (unsigned int)dataBuffer[4] << 8 | dataBuffer[5];
    _temp = ((temp * 200.0) / 1048576.0) - 50;

    // Process humidity
    unsigned int hum = ((unsigned int)dataBuffer[1] << 12) | ((unsigned int)dataBuffer[2] << 4) | ((dataBuffer[3] & 0xF0) >> 4);
    _hum = (hum * 100.0) / 1048576.0;
}

float AHT21Sensor::readTemperature() const {
    return _temp;
}

float AHT21Sensor::readHumidity() const {
    return _hum;
}
