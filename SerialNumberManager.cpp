#include "SerialNumberManager.h"

SerialNumberManager::SerialNumberManager(int startAddress, int length, byte marker)
    : _startAddress(startAddress), _length(length), _marker(marker), _isValid(false) {
    // Ensure that the startAddress and length are within the EEPROM size limits.
}

void SerialNumberManager::begin() {
    EEPROM.begin(512); // Adjust size if needed
    if (!isSerialNumberStored()) {
        generateSerialNumber();
        storeSerialNumber();
    } else {
        readSerialNumber(_serialNumber);
        _isValid = validateSerialNumber(_serialNumber); // Validate after reading
    }
}

bool SerialNumberManager::isSerialNumberStored() {
    // Ensure startAddress is correctly offset to avoid out-of-bounds access
    return EEPROM.read(_startAddress - 1) == _marker;
}

void SerialNumberManager::generateSerialNumber() {
    formatSerialNumber(_serialNumber);
}

void SerialNumberManager::storeSerialNumber() {
    EEPROM.write(_startAddress - 1, _marker);
    for (int i = 0; i < _length; i++) {
        EEPROM.write(_startAddress + i, _serialNumber[i]);
    }
    EEPROM.commit();
}

void SerialNumberManager::readSerialNumber(char *serialNumber) {
    for (int i = 0; i < _length; i++) {
        serialNumber[i] = char(EEPROM.read(_startAddress + i));
    }
    serialNumber[_length] = '\0'; // Ensure null termination
}

void SerialNumberManager::resetSerialNumber() {
    EEPROM.write(_startAddress - 1, 0xFF); // Clear the marker
    for (int i = 0; i < _length; i++) {
        EEPROM.write(_startAddress + i, 0xFF); // Clear each byte of the serial number
    }
    EEPROM.commit();
    _isValid = false; // Invalidate the serial number
}

void SerialNumberManager::formatSerialNumber(char *serialNumber) {
    // Create a random serial number in the specified format
    sprintf(serialNumber, "%08X-%04X-%04X-%04X-%04X%08X",
            random(0, 0xFFFFFFFF), random(0, 0xFFFF), random(0, 0xFFFF),
            random(0, 0xFFFF), random(0, 0xFFFF), random(0, 0xFFFFFFFF));
}

bool SerialNumberManager::validateSerialNumber(const char *serialNumber) {
    // Check the length first
    if (strlen(serialNumber) != 36) {
        return false;
    }

    // Check the positions of '-'
    if (serialNumber[8] != '-' || serialNumber[13] != '-' || serialNumber[18] != '-' || serialNumber[23] != '-') {
        return false;
    }

    // Check each character to be a valid hexadecimal digit
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) continue; // Skip '-' positions
        if (!((serialNumber[i] >= '0' && serialNumber[i] <= '9') || 
              (serialNumber[i] >= 'A' && serialNumber[i] <= 'F') || 
              (serialNumber[i] >= 'a' && serialNumber[i] <= 'f'))) {
            return false;
        }
    }

    return true;
}


bool SerialNumberManager::isValid() {
    return _isValid;
}
