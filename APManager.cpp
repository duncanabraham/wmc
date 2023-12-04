#include "APManager.h"


#define SSID_SIZE 32
#define PASSWORD_SIZE 64
#define MAX_ATTEMPTS 10

APManager::APManager(const char* apSSID, ESP8266WebServer& server, EEPROMConfig& eepromConfig)
    : _apSSID(apSSID), _server(server), _eepromConfig(eepromConfig) {}


void APManager::startAPMode() {
    WiFi.softAP(_apSSID);
    Serial.println("Entered AP Mode. Connect to WiFi network: " + String(_apSSID));

    _server.on("/", HTTP_GET, std::bind(&APManager::handleRoot, this));
    _server.on("/setup", HTTP_POST, std::bind(&APManager::handleSetup, this));

    _server.begin(); // Start the web server

}

void APManager::handleClient() {
    _server.handleClient();
}

void APManager::handleRoot() {
    String html = "<form action='/setup' method='post'>"
                "<input name='ssid' length=32 placeholder='SSID'>"
                "<input type='text' name='password' length=64 placeholder='Password'>"
                "<input type='submit' value='Save'>"
                "</form>";

    _server.send(200, "text/html", html); 
}

void APManager::handleSetup() {
    String ssid = _server.arg("ssid");
    String password = _server.arg("password");

    if (ssid.length() > SSID_SIZE || password.length() > PASSWORD_SIZE) {
      _server.send(400, "text/plain", "SSID or Password too long!");
      return;
    }

    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);

    char ssidCharArray[SSID_SIZE + 1];
    char passwordCharArray[PASSWORD_SIZE + 1];
    ssid.toCharArray(ssidCharArray, sizeof(ssidCharArray));
    password.toCharArray(passwordCharArray, sizeof(passwordCharArray));

    storeCredentials(ssidCharArray, passwordCharArray);

    _server.send(200, "text/plain", "Saved. Restarting...");
    delay(3000);
    ESP.restart(); 
}

void APManager::storeCredentials(char* ssid, char* password) {
    _eepromConfig.writeSSID(ssid);
    _eepromConfig.writePassword(password);
}
