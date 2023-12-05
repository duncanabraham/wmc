#include "MotorController.h"
#include <Wire.h>

#define AS5600_ADDRESS 0x36
#define AS5600_RAW_ANGLE_REG 0x0C // Register for the 12-bit raw angle

#define CALIBRATION_DATA_START 136                 // Start address for calibration data
#define CALIBRATION_DATA_LENGTH sizeof(double) * 2 // Assuming two double values for min and max speeds
#define CALIBRATION_STATE_ADDRESS 152              // An address not used by other data

// Initial PID constants
const double Kp = 2.0; // Proportional gain
const double Ki = 0.5; // Integral gain
const double Kd = 0.1; // Derivative gain

MotorController::MotorController(EEPROMConfig &eepromConfig, AHT21Sensor &aht21Sensor)
    : _eepromConfig(eepromConfig), _aht21Sensor(aht21Sensor), _pid(&_actualSpeed, &_output, &_targetSpeed, Kp, Ki, Kd, DIRECT) {}

double MotorController::getRPM(double speed)
{
  return speed * 60.0 / encoderCountsPerRevolution;
}

void MotorController::init(int rpwmPin, int lpwmPin, int renPin, int lenPin)
{
  _minOperationalSpeed = 0.0; // Default min speed
  _maxOperationalSpeed = 0.0; // Default max speed

  _rpwmPin = rpwmPin;
  _lpwmPin = lpwmPin;
  _renPin = renPin;
  _lenPin = lenPin;

  Wire.begin();

  // Initialize the pins as outputs.
  pinMode(_rpwmPin, OUTPUT);
  pinMode(_lpwmPin, OUTPUT);

  pinMode(_lenPin, OUTPUT);
  pinMode(_renPin, OUTPUT);
  digitalWrite(_lenPin, HIGH);
  digitalWrite(_renPin, HIGH);

  // Initialization code...
  _pid.SetOutputLimits(-255, 255); // Set output limits to match PWM range
  _pid.SetSampleTime(10);          // Set how often the PID loop is updated (in milliseconds)
  _pid.SetMode(AUTOMATIC);         // Set PID to automatic mode

  _lastUpdateTime = millis();
  _lastPosition = 0;

  readGUID(_serialNumber);
  Serial.print("Serial Number: ");
  Serial.println(String(_serialNumber));
  loadCalibrationData();
}

void MotorController::readGUID(char *guid)
{
  _eepromConfig.readGUID(guid);
}

String MotorController::getStatusJson(String FIRMWARE_VERSION, String message)
{
  double currentValue = getRPM(_actualSpeed);
  double targetValue = getRPM(_targetSpeed);

  float temperature = _aht21Sensor.readTemperature();
  float humidity = _aht21Sensor.readHumidity();

  String json = "{";
  json += "\"firmwareVersion\":\"" + FIRMWARE_VERSION + "\",";
  json += "\"serialNumber\":\"" + String(_serialNumber) + "\",";
  json += "\"calibrated\":" + String(!_isCalibrated ? "true" : "false") + ",";
  json += "\"pid\":{\"kp\":" + String(Kp) + ",\"ki\":" + String(Ki) + ",\"kd\":" + String(Kd) + "},";
  json += "\"direction\":\"" + _direction + "\",";
  json += "\"minSpeed\":" + String(_minOperationalSpeed) + ",";
  json += "\"maxSpeed\":" + String(_maxOperationalSpeed) + ",";
  json += "\"position\":" + String(currentPosition) + ",";
  json += "\"actualSpeed\":" + String(_actualSpeed) + ",";
  json += "\"targetSpeed\":" + String(_targetSpeed) + ",";
  json += "\"actualSpeedRPM\":" + String(currentValue) + ",";
  json += "\"targetSpeedRPM\":" + String(targetValue) + ",";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"message\":\"" + String(message) + "\"";
  json += "}";
  return json;
}

void MotorController::hold()
{
  _holdPosition = readEncoder(); // Capture the current position
  _isHolding = true;             // Set the flag to indicate hold mode
}

void MotorController::brake()
{
  _isHolding = false;
  digitalWrite(_lenPin, HIGH);
  digitalWrite(_renPin, HIGH);
  digitalWrite(_lpwmPin, HIGH);
  digitalWrite(_rpwmPin, HIGH);
}

void MotorController::release()
{
  _isHolding = false;
  digitalWrite(_lenPin, LOW);
  digitalWrite(_renPin, LOW);
  digitalWrite(_lpwmPin, LOW);
  digitalWrite(_rpwmPin, LOW);
}

void MotorController::free()
{
  _isHolding = false;
  analogWrite(_rpwmPin, 0);
  analogWrite(_lpwmPin, 0);
  digitalWrite(_lenPin, LOW);
  digitalWrite(_renPin, LOW);
}

int MotorController::readEncoder()
{
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_RAW_ANGLE_REG);
  Wire.endTransmission(false);

  Wire.requestFrom(AS5600_ADDRESS, 2);
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

void MotorController::update()
{
  unsigned long currentTime = millis();
  unsigned long timeChange = (currentTime - _lastUpdateTime);

  currentPosition = readEncoder();
  if (timeChange >= SampleTime)
  {
    if (_isHolding)
    {
      // In hold mode, adjust the target speed based on the difference from the hold position
      int scalingFactor = 1;
      int positionError = _holdPosition - currentPosition;
      setTargetSpeed(positionError * scalingFactor); // someScalingFactor to be determined based on system
    }

    // Read the encoder position
    int newPosition = currentPosition; // Implement this function based on your encoder

    // Calculate speed as a difference in position over time
    //  _actualSpeed = (newPosition - _lastPosition) / timeChange * 1000.0; // Converts to units per second
    _actualSpeed = static_cast<double>(newPosition - _lastPosition) / timeChange * 1000.0;

    // Update the PID controller
    _pid.Compute();

    // Update motor PWM based on PID output
    bool isForward = _output >= 0;
    int pwmValue = map(abs(_output), 0, 255, 0, 255);

    if (isForward)
    {
      analogWrite(_rpwmPin, pwmValue);
      analogWrite(_lpwmPin, 0);
    }
    else
    {
      analogWrite(_rpwmPin, 0);
      analogWrite(_lpwmPin, pwmValue);
    }

    // Save position and time for the next update
    _lastPosition = newPosition;
    _lastUpdateTime = currentTime;
  }
}

void MotorController::setDirection(String direction)
{
  _direction = direction;
}

void MotorController::setSpeed(double rpm)
{
  if (rpm < _minOperationalSpeed || rpm > _maxOperationalSpeed)
  {
    _isHolding = false;

    double targetEncoderCountsPerSecond = rpmToEncoderCountsPerSecond(rpm);
    setTargetSpeed(targetEncoderCountsPerSecond);
    setDirection(rpm > 0 ? "CW" : rpm < 0 ? "CCW"
                                          : "STOPPED");
  }
}

void MotorController::setTargetSpeed(double speed)
{
  _targetSpeed = speed;
  _pid.SetMode(AUTOMATIC); // Enable the PID controller if not already enabled
  digitalWrite(_lenPin, HIGH);
  digitalWrite(_renPin, HIGH);
}

double MotorController::rpmToEncoderCountsPerSecond(double rpm)
{
  const double encoderCountsPerRevolution = 4096; // 12-bit encoder
  return (rpm / 60.0) * encoderCountsPerRevolution;
}

void MotorController::saveCalibrationData()
{
  _eepromConfig.writeMinOperationalSpeed(_minOperationalSpeed);
  _eepromConfig.writeMaxOperationalSpeed(_maxOperationalSpeed);
  _eepromConfig.writeCalibrationState(false); // Writing false to indicate calibration is set

  // No need to call commit here if writeConfig in EEPROMConfig already does it
}

void MotorController::loadCalibrationData()
{
  // Check if calibration data exists
  _isCalibrated = _eepromConfig.readCalibrationState();

  if (!_isCalibrated)
  {
    _minOperationalSpeed = _eepromConfig.readMinOperationalSpeed();
    _maxOperationalSpeed = _eepromConfig.readMaxOperationalSpeed();
  }
  else
  {
    // If calibration data does not exist, use the default values
    _minOperationalSpeed = 0.0;
    _maxOperationalSpeed = 0.0;
  }
}

void MotorController::clearEEPROM()
{
  _eepromConfig.clearEEPROM();
}

float MotorController::calculateRpm(int startPosition, int endPosition, unsigned long timeMillis)
{
  int countDifference = abs(endPosition - startPosition);
  double revolutions = static_cast<double>(countDifference) / encoderCountsPerRevolution;
  double timeMinutes = static_cast<double>(timeMillis) / 60000.0; // Convert milliseconds to minutes
  double rpm = revolutions / timeMinutes;
  return static_cast<float>(rpm);
}

void MotorController::calibrate()
{
  const int maxAttempts = 100; // Set an appropriate limit
  int calibrationDelay = 50;
  float maxTestSpeed = 10.0;
  float speedIncrement = 0.1;
  _minOperationalSpeed = maxTestSpeed;
  int startPosition = readEncoder();
  bool movementDetected = false;
  int attempts = 0;

  for (float speed = 0.1; speed <= maxTestSpeed && attempts < maxAttempts; speed += speedIncrement)
  {
    setSpeed(speed);
    delay(calibrationDelay);
    attempts++;

    currentPosition = readEncoder();
    if (abs(currentPosition - startPosition) > 2)
    {
      _minOperationalSpeed = speed;
      movementDetected = true;
      break;
    }
  }

  // Data collection for curve fitting
  std::vector<float> pwmPercentages = {0.1, 0.2, 0.3, 0.4, 0.5}; // 10%, 20%, 30%, 40%, 50% of PWM
  std::vector<float> recordedRpms;

  for (float pwmPercentage : pwmPercentages)
  {
    float pwmValue = pwmPercentage * 255.0; // Convert percentage to PWM value
    analogWrite(_rpwmPin, pwmValue);        // Set motor speed
    delay(1000);                            // Delay to stabilize speed

    currentPosition = readEncoder();
    float currentRpm = calculateRpm(_lastPosition, currentPosition, 5000);
    recordedRpms.push_back(currentRpm);

    _lastPosition = currentPosition; // Update last position for next measurement
  }

  if (!movementDetected)
  {
    // Handle the case where no movement was detected within the test range
    _minOperationalSpeed = 0;
  }

  saveCalibrationData();
}
