#ifndef APManager_h
#define APManager_h

#include <ESP8266WebServer.h>
#include <Arduino.h>
#include "EEPROMConfig.h"

class APManager {
public:
    APManager(const char* apSSID, ESP8266WebServer& server, EEPROMConfig& eepromConfig);
    void startAPMode();
    void handleClient();

private:
    const char* _apSSID;
    ESP8266WebServer& _server;
    EEPROMConfig& _eepromConfig;

    void handleRoot();
    void handleReset();
    void handleSetup();
    void storeCredentials(char* ssid, char* password);
    void resetGUID();
};

#endif
