# Changelog

0.1.3 - Encoder as a task
* Encoder runs all the time and can be queried
* Started PID tuning
* Fixed issue with motors not running

0.1.2 - Some new API commands
* Ensure enable pins are high
* Added "brake" command
* Address "release" command

0.1.1 - Moved eeprom tasks into a new class
* Moved eeprom read and write functions into a new class
* Updated references to eeprom to use the new class
* Added AHT21 Temperature and Humidity sensor

0.1.0 - Fixed issue with AP mode when the network is not recognised
* Return status after hold, free and speed commands
* Fixed AP mode on new network
* Added message to JSON data
* Changed the Motor controller name to include the serial number

0.0.2 - Adding new functionality that was provided by a 2nd module originally
* AP mode when not configured
* Serial Number generator

0.0.1 - All the core functionality was built under this version which includes:
* Motor Control
* API Commands
* Calibration Routine
* OTA Mode
* Data logging
* Speeds in RPM