#ifndef MotorController_h
#define MotorController_h

#include "Arduino.h"
#include <PID_v1.h>
#include "AHT21Sensor.h"
#include "EEPROMConfig.h"
#include "Encoder.h"

#define GUID_LENGTH 36                // Length of the GUID string
#define GUID_START 100                // EEPROM address to store the GUID
#define GUID_MARKER 0xAA              // Example marker value
#define MARKER_START (GUID_START - 1) // Assuming there's a byte space before GUID_START

class MotorController
{
public:
    MotorController(EEPROMConfig &eepromConfig, AHT21Sensor &aht21Sensor, Encoder &encoder);
    void init(int rpwmPin, int lpwmPin, int renPin, int lenPin);
    void setTargetSpeed(double speed);
    void hold();
    void free();
    void brake();
    void release();
    void update();    // Make this public so it can be called from loop()
    void calibrate(); // Method for calibration
    void factoryReset();

    void setPIDParameters(double Kp, double Ki, double Kd);
    void clearEEPROM();

    String getStatusJson(String FIRMWARE_VERSION, String message);

private:
    int _rpwmPin; // Right PWM pin
    int _lpwmPin; // Left PWM pin
    int _lenPin; // Left Enable pin 
    int _renPin; // Right Enable pin

    double _targetSpeed; // Target speed set by the user
    double _actualSpeed; // Actual speed read from the encoder
    double _output;      // Output to the motor driver
    
    double _targetSpeedRPM; // Set value by user API

    unsigned long _lastUpdateTime; // Time of the last PID update
    int _lastPosition;             // Last position read from the encoder
    int SampleTime = 5;          // Sample time in milliseconds for PID update
    char _serialNumber[37];

    int _holdPosition; // Target position for holding
    bool _isHolding;   // Flag to indicate if the motor is in hold mode
    int currentPosition; // raw value from the encoder

    double _minOperationalSpeed; // Minimum operational speed
    double _maxOperationalSpeed; // Maximum operational speed

    bool _isCalibrated; 

    String _direction;

    PID _pid; // PID controller object

    const int encoderCountsPerRevolution = 4096;

    AHT21Sensor &_aht21Sensor;
    EEPROMConfig &_eepromConfig;
    Encoder &_encoder;

    int readEncoder(); // Method to read the encoder position
    double rpmToEncoderCountsPerSecond(double rpm);
    void readGUID(char *guid);
    void setDirection(String direction);
    double getRPM(double speed);
    void saveCalibrationData(); // Save calibration data to EEPROM
    void loadCalibrationData(); // Load calibration data from EEPROM
    float calculateRpm(int startPosition, int endPosition, unsigned long timeMillis);
    void updateMotorPWM(double output);
    float estimateMaxSpeed(const std::vector<float>& pwmPercentages, const std::vector<float>& recordedRpms);
    double rpmToPWM(double rpm);
    double pwmToRPM(double speed);
};

#endif
