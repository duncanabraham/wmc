#ifndef SerialNumberManager_h
#define SerialNumberManager_h

#include <EEPROM.h>
#include <Arduino.h>

class SerialNumberManager {
public:
    SerialNumberManager(int startAddress, int length, byte marker);
    void begin();
    bool isSerialNumberStored();
    void generateSerialNumber();
    void storeSerialNumber();
    void readSerialNumber(char *serialNumber);
    void resetSerialNumber();
    bool isValid();

private:
    int _startAddress;
    int _length;
    byte _marker;
    char _serialNumber[37]; // Enough space for a 36-character serial number + null termination
    bool _isValid;

    void formatSerialNumber(char *serialNumber);
    bool validateSerialNumber(const char *serialNumber);
};

#endif
