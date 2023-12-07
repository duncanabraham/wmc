#include "MotorController.h"

#define AS5600_ADDRESS 0x36
#define AS5600_RAW_ANGLE_REG 0x0C // Register for the 12-bit raw angle

#define CALIBRATION_DATA_START 136                 // Start address for calibration data
#define CALIBRATION_DATA_LENGTH sizeof(double) * 2 // Assuming two double values for min and max speeds
#define CALIBRATION_STATE_ADDRESS 152              // An address not used by other data

double easeInOut(double currentTime, double startValue, double changeInValue, double duration)
{
  currentTime /= duration / 2;
  if (currentTime < 1)
    return changeInValue / 2 * currentTime * currentTime + startValue;
  currentTime--;
  return -changeInValue / 2 * (currentTime * (currentTime - 2) - 1) + startValue;
}

MotorController::MotorController(EEPROMConfig &eepromConfig, AHT21Sensor &aht21Sensor, Encoder &encoder)
    : _eepromConfig(eepromConfig), _aht21Sensor(aht21Sensor), _encoder(encoder), _kp(2.0), _ki(0.1), _kd(0.1), _pid(&_actualSpeed, &_output, &_targetSpeed, _kp, _ki, _kd, DIRECT)
{
  // ... rest of the constructor ...
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
  _pid.SetOutputLimits(-1023, 1023); // Set output limits to match PWM range
  _pid.SetSampleTime(SampleTime);    // Set how often the PID loop is updated (in milliseconds)
  _pid.SetMode(AUTOMATIC);           // Set PID to automatic mode

  _lastUpdateTime = millis();
  _lastPosition = 0;

  readGUID(_serialNumber);
  Serial.print("Serial Number: ");
  Serial.println(String(_serialNumber));
  loadCalibrationData();
}

void MotorController::setPIDValues(double kp, double ki, double kd)
{
  _kp = kp;
  _ki = ki;
  _kd = kd;
  _pid.SetTunings(_kp, _ki, _kd);
}
void MotorController::readGUID(char *guid)
{
  _eepromConfig.readGUID(guid);
}

String MotorController::getStatusJson(String FIRMWARE_VERSION, String message)
{
  double currentValue = _encoder.getSpeed();

  float temperature = _aht21Sensor.readTemperature();
  float humidity = _aht21Sensor.readHumidity();

  String json = "{";
  json += "\"firmwareVersion\":\"" + FIRMWARE_VERSION + "\",";
  json += "\"serialNumber\":\"" + String(_serialNumber) + "\",";
  json += "\"calibrated\":" + String(!_isCalibrated ? "true" : "false") + ",";
  json += "\"pid\":{\"kp\":" + String(_kp) + ",\"ki\":" + String(_ki) + ",\"kd\":" + String(_kd) + "},";
  json += "\"direction\":\"" + _direction + "\",";
  json += "\"minSpeed\":" + String(_minOperationalSpeed) + ",";
  json += "\"maxSpeed\":" + String(_maxOperationalSpeed) + ",";
  json += "\"position\":" + String(currentPosition) + ",";
  json += "\"actualSpeed\":" + String(_actualSpeed) + ",";
  json += "\"targetSpeed\":" + String(_targetSpeed) + ",";
  json += "\"actualSpeedRPM\":" + String(currentValue) + ",";
  json += "\"targetSpeedRPM\":" + String(_targetSpeedRPM) + ",";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"message\":\"" + String(message) + "\"";
  json += "}";
  return json;
}

void MotorController::hold()
{
  _holdPosition = _encoder.readRawAngle(); // Capture the current position
  setTargetSpeed(0);
  _isHolding = true; // Set the flag to indicate hold mode
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

double MotorController::rpmToPWM(double rpm)
{
  const int maxRPM = 3500; // Maximum RPM
  const int maxPWM = 1023; // Maximum PWM value
  // Calculate percentage of max speed (this could be negative if rpm is negative)
  double percSpeed = rpm / maxRPM;
  // Scale the percentage to PWM range and clamp the value between -maxPWM and maxPWM
  int pwmValue = static_cast<int>(maxPWM * percSpeed);
  pwmValue = max(-maxPWM, min(pwmValue, maxPWM)); // Ensuring the PWM is within range
  return pwmValue;
}

void MotorController::update()
{
  unsigned long currentTime = millis();
  unsigned long timeChange = (currentTime - _lastUpdateTime);

  if (timeChange >= SampleTime)
  {
    currentPosition = _encoder.readRawAngle();
    double currentSpeedRPM = _encoder.getSpeed();

    // Use _encoder.getTotalRevolutions() if you need total revolutions count

    if (_isHolding)
    {
      // Adjust target speed based on the difference from the hold position
      int positionError = _holdPosition - currentPosition;
      setTargetSpeed(positionError);
    }

    // Update the PID controller
    _actualSpeed = rpmToPWM(currentSpeedRPM);
    _direction = _encoder.getDirection();

    _pid.Compute();

    // Update motor PWM based on PID output
    updateMotorPWM(_output);

    // Save time for the next update
    _lastUpdateTime = currentTime;
  }
}

void MotorController::updateMotorPWM(double output)
{
  bool isForward = output >= 0;
  int pwmValue = map(abs(output), 0, 1023, 0, 1023);

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
}

void MotorController::setDirection(String direction)
{
  _direction = direction;
}

void MotorController::setTargetSpeed(double speed) // pass the speed as RPM but remember the PID works between -255 and +255
{
  _targetSpeedRPM = speed;
  _actualSpeed = 0;
  _targetSpeed = rpmToPWM(speed);
  _pid.SetMode(AUTOMATIC); // Enable the PID controller if not already enabled
  digitalWrite(_lenPin, HIGH);
  digitalWrite(_renPin, HIGH);
  setDirection(speed > 0 ? "CW" : speed < 0 ? "CCW"
                                            : "STOPPED");
}

double MotorController::rpmToEncoderCountsPerSecond(double rpm)
{
  const double encoderCountsPerRevolution = 4096; // 12-bit encoder
  return (rpm / 60.0) * encoderCountsPerRevolution;
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

void MotorController::saveCalibrationData()
{
  _eepromConfig.writeMinOperationalSpeed(_minOperationalSpeed);
  _eepromConfig.writeMaxOperationalSpeed(_maxOperationalSpeed);
  _eepromConfig.writeCalibrationState(false);
}

void MotorController::calibrate()
{
  // make sure the controller is on
  digitalWrite(_lenPin, HIGH);
  digitalWrite(_renPin, HIGH);

  const int maxAttempts = 100; // Set an appropriate limit
  int calibrationDelay = 50;
  float maxTestSpeed = 600.0;
  float speedIncrement = 1;
  _minOperationalSpeed = maxTestSpeed;
  int startPosition = _encoder.readRawAngle();
  bool movementDetected = false;
  int attempts = 0;

  for (float speed = 0.0; speed <= maxTestSpeed && attempts < maxAttempts; speed += speedIncrement)
  {
    float pwmValue = rpmToPWM(speed); // Convert percentage to PWM value
    analogWrite(_rpwmPin, pwmValue);  // Set motor speed
    delay(calibrationDelay);
    attempts++;
    _encoder.update();
    currentPosition = _encoder.readRawAngle();
    if (abs(currentPosition - startPosition) > 300)
    {
      _minOperationalSpeed = speed;
      movementDetected = true;
      break;
    }
  }

  // Data collection for curve fitting
  std::vector<float> pwmPercentages = {0.2, 0.3, 0.4, 0.5}; // Example percentages
  std::vector<float> recordedRpms;
  
  float currentRpm = 0;
  for (float pwmPercentage : pwmPercentages) {
    float pwmValue = pwmPercentage * 1023.0; // Scale percentage to PWM value
    analogWrite(_rpwmPin, pwmValue);         // Set motor speed

    unsigned long startTime = millis();
    unsigned long lastUpdateTime = 0;
    while (millis() - startTime < 1000) {    // Loop for 1 second
      unsigned long currentTime = millis();
      if (currentTime - lastUpdateTime >= 5) { // Every 5ms
        _encoder.update();                      // Update encoder reading
        lastUpdateTime = currentTime;
      }
    }

    _encoder.update(); // One final update after the stabilization period
    float currentRpm = _encoder.getSpeed();
    recordedRpms.push_back(currentRpm);
  }

  // Analyze the recorded RPM data to estimate the maximum speed
  _maxOperationalSpeed = currentRpm * 2; // estimateMaxSpeed(pwmPercentages, recordedRpms);
  
  analogWrite(_rpwmPin, 0);
  digitalWrite(_lenPin, LOW);
  digitalWrite(_renPin, LOW);
  saveCalibrationData();
}

// Fit the measured speeds to a straight line estimation
float MotorController::estimateMaxSpeed(const std::vector<float> &pwmPercentages, const std::vector<float> &recordedRpms)
{
  if (pwmPercentages.size() != recordedRpms.size() || pwmPercentages.empty())
  {
    // Handle error: The vectors must be of the same size and not empty
    return 0;
  }

  float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
  int n = pwmPercentages.size();

  for (int i = 0; i < n; i++)
  {
    sumX += pwmPercentages[i];
    sumY += recordedRpms[i];
    sumXY += pwmPercentages[i] * recordedRpms[i];
    sumX2 += pwmPercentages[i] * pwmPercentages[i];
  }

  // Calculating the coefficients
  float xMean = sumX / n;
  float yMean = sumY / n;
  float slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
  float intercept = yMean - slope * xMean;

  // Assuming the maximum speed is where the line intersects the maximum PWM value (100%)
  float maxPwmPercentage = 1.0; // 100%
  float estimatedMaxSpeed = slope * maxPwmPercentage + intercept;

  return estimatedMaxSpeed;
}
