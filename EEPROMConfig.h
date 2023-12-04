#ifndef EEPROMConfig_h
#define EEPROMConfig_h

#include <Arduino.h>
#include <EEPROM.h>

class EEPROMConfig {
public:
  EEPROMConfig();

  void begin();
  void clearEEPROM();

  void readSSID(char* ssid);
  void writeSSID(char* SSID);

  void readPassword(char* password);
  void writePassword(char* password);

  void readGUID(char* guid);
  void writeGUID(const char* guid);

  float readTemperatureCutoff();
  void writeTemperatureCutoff(float temp);

  int readMaxRPM();
  void writeMaxRPM(int rpm);

  float readVoltageCutoff();
  void writeVoltageCutoff(float voltage);

  String readDeviceName();
  void writeDeviceName(const String& name);

  double readMinOperationalSpeed();
  void writeMinOperationalSpeed(double speed);

  double readMaxOperationalSpeed();
  void writeMaxOperationalSpeed(double speed);

  bool readCalibrationState();
  void writeCalibrationState(bool state);

private:
  const int SSID_START = 0;
  const int SSID_SIZE = 32;
  const int PASSWORD_START = SSID_START + SSID_SIZE;
  const int PASSWORD_SIZE = 64;

  const int GUID_START = 100;
  const int GUID_LENGTH = 36;

  const int TEMP_CUTOFF_ADDR = GUID_START + GUID_LENGTH + 1;
  const int RPM_LIMIT_ADDR = TEMP_CUTOFF_ADDR + sizeof(float);
  const int VOLT_CUTOFF_ADDR = RPM_LIMIT_ADDR + sizeof(int);
  const int DEVICE_NAME_ADDR = VOLT_CUTOFF_ADDR + sizeof(float);
  const int DEVICE_NAME_LENGTH = 20;
  const int MIN_OPERATIONAL_SPEED_ADDR = DEVICE_NAME_ADDR + DEVICE_NAME_LENGTH + 1;
  const int MAX_OPERATIONAL_SPEED_ADDR = MIN_OPERATIONAL_SPEED_ADDR + sizeof(double);
  const int CALIBRATION_STATE_ADDR = MAX_OPERATIONAL_SPEED_ADDR + sizeof(double);
};

// Template function definitions
template<typename T>
T readData(int address) {
  T value;
  EEPROM.get(address, value);
  return value;
}

template<typename T>
void writeData(int address, const T& value) {
  EEPROM.put(address, value);
  EEPROM.commit();
}

#endif
