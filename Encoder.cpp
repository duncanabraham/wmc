#include "Encoder.h"
#include <Wire.h>

#define AS5600_RAW_ANGLE_REG 0x0C // Register for the 12-bit raw angle

Encoder::Encoder(uint8_t i2cAddress) : _i2cAddress(i2cAddress)
{
  _lastRawAngle = 0;
  _totalRevolutions = 0;
  _speed = 0.0;
  _lastUpdateTime = millis();
}

void Encoder::begin()
{
  Wire.begin();
  _lastSpeed = 0;
  _lastRawAngle = readRawAngle(); // Initial reading
}

int Encoder::readRawAngle()
{
  Wire.beginTransmission(_i2cAddress);
  Wire.write(AS5600_RAW_ANGLE_REG);
  Wire.endTransmission(false);

  Wire.requestFrom(static_cast<int>(_i2cAddress), 2);
  if (Wire.available() == 2)
  {
    uint16_t rawAngle = Wire.read() << 8;
    rawAngle |= Wire.read();
    return static_cast<int>(rawAngle);
  }
  else
  {
    return -1; // Indicate an error
  }
}

void Encoder::update() {
  unsigned long currentTime = millis();
  int currentRawAngle = readRawAngle();

  if (currentRawAngle != -1) {
    // Handle wraparound
    int angleDifference = currentRawAngle - _lastRawAngle;
    if (angleDifference > 2048) { // More than half the encoder range in the negative direction
      angleDifference -= 4096; // Adjust for wraparound in the negative direction
    } else if (angleDifference < -2048) { // More than half the encoder range in the positive direction
      angleDifference += 4096; // Adjust for wraparound in the positive direction
    }

    unsigned long timeDifference = currentTime - _lastUpdateTime;

    // Ensure time difference is not zero to avoid division by zero
    if (timeDifference > 0) {
      // Calculate speed in ticks per millisecond, and then convert to ticks per second
      float newSpeed = (angleDifference / static_cast<float>(timeDifference)) * 1000.0;
      // Simple low-pass filter to smooth out speed reading
      _speed = _speed * 0.9 + newSpeed * 0.1;
    }

    // Update total revolutions and direction
    if (angleDifference > 0) {
      _totalRevolutions += angleDifference;
      _direction = "CW";
    } else if (angleDifference < 0) {
      _totalRevolutions += angleDifference; // This will decrement since angleDifference is negative
      _direction = "CCW";
    }

    _lastRawAngle = currentRawAngle;
    _lastUpdateTime = currentTime;
  }
}


// void Encoder::update()
// {
//   unsigned long currentTime = millis();
//   int currentRawAngle = readRawAngle();

//   if (currentRawAngle != -1 && _lastRawAngle != -1)
//   {
//     int angleDifference = currentRawAngle - _lastRawAngle;
//     unsigned long timeDifference = currentTime - _lastUpdateTime;

//     // Ensure time difference is not zero to avoid division by zero
//     if (timeDifference > 0)
//     {
//       // Calculate speed in ticks per millisecond, and then convert to ticks per second
//       _speed = (angleDifference / static_cast<float>(timeDifference)) * 1000.0;
//     }

//     if (angleDifference > 1)
//     {
//       // Forward movement
//       _totalRevolutions++;
//       _direction = "CW";
//     }
//     else if (angleDifference < -1)
//     {
//       // Reverse movement
//       _totalRevolutions--;
//       _direction = "CCW";
//     }

//     _lastRawAngle = currentRawAngle;
//     _lastUpdateTime = currentTime;
//   }
// }

long Encoder::getTotalRevolutions()
{
  return _totalRevolutions;
}

float Encoder::getSpeed()
{
  const int ticksPerRevolution = 4096;
  float newSpeed = (_speed / ticksPerRevolution) * 60.0 * -1; // invert the reading as I think out + and - speeds are backwards
  return newSpeed;
}

String Encoder::getDirection()
{
  return _direction;
}
