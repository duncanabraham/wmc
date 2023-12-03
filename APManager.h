#ifndef APManager_h
#define APManager_h

#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Arduino.h>

class APManager {
public:
    APManager(const char* apSSID, ESP8266WebServer& server);
    void startAPMode();
    void handleClient();

private:
    const char* _apSSID;
    ESP8266WebServer& _server;

    void handleRoot();
    void handleReset();
    void handleSetup();
    void storeCredentials(const char* ssid, const char* password);
    void resetGUID();
};

#endif
