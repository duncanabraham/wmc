#include "EEPROMConfig.h"

EEPROMConfig::EEPROMConfig() {
  // Constructor
}

void EEPROMConfig::begin() {
  EEPROM.begin(512);  // Adjust size if needed
}

void EEPROMConfig::clearEEPROM() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
}

void EEPROMConfig::writeSSID(char* ssid) {
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(SSID_START + i, ssid[i]);
  }
  EEPROM.commit();
}

void EEPROMConfig::readSSID(char* ssid) {
  for (int i = 0; i < 32; ++i) {
    ssid[i] = EEPROM.read(SSID_START + i);
  }
  ssid[32] = '\0';  // Ensure null termination
}

void EEPROMConfig::writePassword(char* password) {
  for (int i = 0; i < 64; ++i) {
    EEPROM.write(PASSWORD_START + i, password[i]);
  }
  EEPROM.commit();
}

void EEPROMConfig::readPassword(char* password) {
  for (int i = 0; i < 64; ++i) {
    password[i] = EEPROM.read(PASSWORD_START + i);
  }
  password[64] = '\0';  // Ensure null termination
}

void EEPROMConfig::writeGUID(const char* guid) {
  for (int i = 0; i < GUID_LENGTH; ++i) {
    EEPROM.write(GUID_START + i, guid[i]);
  }
  EEPROM.commit();
}

void EEPROMConfig::readGUID(char* guid) {
  for (int i = 0; i < GUID_LENGTH; ++i) {
    guid[i] = EEPROM.read(GUID_START + i);
  }
  guid[GUID_LENGTH] = '\0';  // Ensure null termination
}

void EEPROMConfig::writeTemperatureCutoff(float temp) {
  writeData<float>(TEMP_CUTOFF_ADDR, temp);
}

float EEPROMConfig::readTemperatureCutoff() {
  return readData<float>(TEMP_CUTOFF_ADDR);
}

void EEPROMConfig::writeMaxRPM(int rpm) {
  writeData<int>(RPM_LIMIT_ADDR, rpm);
}

int EEPROMConfig::readMaxRPM() {
  return readData<int>(RPM_LIMIT_ADDR);
}

void EEPROMConfig::writeVoltageCutoff(float voltage) {
  writeData<float>(VOLT_CUTOFF_ADDR, voltage);
}

float EEPROMConfig::readVoltageCutoff() {
  return readData<float>(VOLT_CUTOFF_ADDR);
}

void EEPROMConfig::writeDeviceName(const String& name) {
  for (int i = 0; i < DEVICE_NAME_LENGTH; ++i) {
    EEPROM.write(DEVICE_NAME_ADDR + i, i < name.length() ? name[i] : 0);
  }
  EEPROM.commit();
}

String EEPROMConfig::readDeviceName() {
  char name[DEVICE_NAME_LENGTH + 1];
  for (int i = 0; i < DEVICE_NAME_LENGTH; ++i) {
    name[i] = EEPROM.read(DEVICE_NAME_ADDR + i);
  }
  name[DEVICE_NAME_LENGTH] = '\0';  // Ensure null termination
  return String(name);
}

void EEPROMConfig::writeMinOperationalSpeed(double speed) {
  writeData<double>(MIN_OPERATIONAL_SPEED_ADDR, speed);
}

double EEPROMConfig::readMinOperationalSpeed() {
  return readData<double>(MIN_OPERATIONAL_SPEED_ADDR);
}

void EEPROMConfig::writeMaxOperationalSpeed(double speed) {
  writeData<double>(MAX_OPERATIONAL_SPEED_ADDR, speed);
}

double EEPROMConfig::readMaxOperationalSpeed() {
  return readData<double>(MAX_OPERATIONAL_SPEED_ADDR);
}

void EEPROMConfig::writeCalibrationState(bool state) {
  writeData<bool>(CALIBRATION_STATE_ADDR, state);
}

bool EEPROMConfig::readCalibrationState() {
  return readData<bool>(CALIBRATION_STATE_ADDR);
}
