#ifndef Encoder_h
#define Encoder_h

#include <Arduino.h>

class Encoder {
public:
    Encoder(uint8_t i2cAddress);
    void begin();
    int readRawAngle();
    void update();
    long getTotalRevolutions();
    float getSpeed();
    String getDirection();

private:
    uint8_t _i2cAddress;
    int _lastRawAngle;
    long _totalRevolutions;
    float _speed;
    unsigned long _lastUpdateTime;
    String _direction;
    float _lastSpeed;
};

#endif
