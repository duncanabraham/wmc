#include "ServerManager.h"

ServerManager::ServerManager(ESP8266WebServer& server, MotorController& motorController, String FIRMWARE_VERSION)
    : _server(server), _motorController(motorController), _FIRMWARE_VERSION(FIRMWARE_VERSION) {}

void ServerManager::setupEndpoints() {
    _server.on("/hold", HTTP_GET, std::bind(&ServerManager::handleHold, this));
    _server.on("/speed", HTTP_GET, std::bind(&ServerManager::handleSpeed, this));
    _server.on("/free", HTTP_GET, std::bind(&ServerManager::handleFree, this));
    _server.on("/brake", HTTP_GET, std::bind(&ServerManager::handleBrake, this));
    _server.on("/release", HTTP_GET, std::bind(&ServerManager::handleRelease, this));
    _server.on("/status", HTTP_GET, std::bind(&ServerManager::handleStatus, this));
    _server.on("/calibrate", HTTP_GET, std::bind(&ServerManager::handleCalibrate, this));
    _server.on("/factory_reset", HTTP_GET, std::bind(&ServerManager::handleFactoryReset, this));
    _server.on("/config", HTTP_GET, std::bind(&ServerManager::handleConfig, this));
    _server.on("/setup", HTTP_POST, std::bind(&ServerManager::handleSetup, this));
    _server.begin();
}

void ServerManager::handleClient() {
    _server.handleClient();
}

void ServerManager::handleHold() {
    _motorController.hold();
    String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION, "Hold Set");
    _server.send(200, "application/json", statusJson);
}

void ServerManager::handleSpeed() {
    if (_server.hasArg("value")) {
        int speed = _server.arg("value").toInt(); // Assumes speed values are passed as query parameters.
        _motorController.setSpeed(speed);
        String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION, "Speed Set");
        _server.send(200, "application/json", statusJson);
    } else {
        _server.send(400, "text/plain", "Speed value not provided.");
    }
}

void ServerManager::handleFree() {
    _motorController.free();
    String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION, "Free Set");
    _server.send(200, "application/json", statusJson);
}

void ServerManager::handleBrake() {
    _motorController.brake();
    String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION, "Brake Applied");
    _server.send(200, "application/json", statusJson);
}

void ServerManager::handleRelease() {
    _motorController.release();
    String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION, "Brake Released");
    _server.send(200, "application/json", statusJson);
}

void ServerManager::handleStatus() {
    String statusJson = _motorController.getStatusJson(_FIRMWARE_VERSION,"");
    _server.send(200, "application/json", statusJson);
}

void ServerManager::handleCalibrate() {
    _motorController.calibrate();
    _server.send(200, "text/plain", "Motor Calibration Complete");
}

void ServerManager::handleFactoryReset() {
    _motorController.clearEEPROM();
    _server.send(200, "text/plain", "Motor Controller reset to defaults");
    delay(3000);
    ESP.restart();
}

void ServerManager::handleConfig() {
    String html = "<!DOCTYPE html><html><head>"
                  "<title>Motor Controller Configuration</title>"
                  "<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css'>"
                  "</head><body>"
                  "<div class='container'>"
                  "  <h2>Motor Controller Configuration</h2>"
                  "  <form action='/setup' method='post' class='form-group'>"
                  "    <div class=\"row\">"
                  "      <div class=\"col-6\"><label>SSID</label>"
                  "        <input class='form-control mb-2' name='ssid' value='' length=32 placeholder='SSID'>"
                  "      </div>"
                  "      <div class=\"col-6\"><label>Password</label>"
                  "        <input class='form-control mb-2' type='text' name='password' length=64 placeholder='Password'>"
                  "      </div>"
                  "    </div>"
                //   "    <div class=\"row\">"
                //   "      <div class=\"col-12\"><label>Name (short name to identify this motor controller)</label>"
                //   "        <input class='form-control mb-2' name='name' length=32 placeholder='Name'>"
                //   "      </div>"
                //   "    </div>"
                  
                  "    <div class=\"row right\">"
                  "      <input type='submit' value='Save' class='btn btn-primary'>"
                  "    </div>"
                  "  </form></div></body></html>";

    _server.send(200, "text/html", html); 
}


void ServerManager::handleSetup() {
    _motorController.clearEEPROM();
    _server.send(200, "text/plain", "Motor Controller setup saved - not implemented yet");
}
