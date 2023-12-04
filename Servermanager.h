#ifndef ServerManager_h
#define ServerManager_h

#include <ESP8266WebServer.h>
#include "MotorController.h"

class ServerManager {
public:
    ServerManager(ESP8266WebServer& server, MotorController& motorController, String FIRMWARE_VERSION);
    void setupEndpoints();
    void handleClient();

private:
    ESP8266WebServer& _server;
    MotorController& _motorController;
    String _FIRMWARE_VERSION;

    void handleHold();
    void handleSpeed();
    void handleFree();
    void handleStatus();
    void handleCalibrate();
    void handleFactoryReset();
    void handleConfig();
    void handleSetup();
};

#endif
