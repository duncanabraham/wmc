#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "MotorController.h"
#include "SerialNumberManager.h"
#include "APManager.h"
#include "ServerManager.h"
#include "EEPROMConfig.h"
#include "AHT21Sensor.h"
#include "Encoder.h"

#define SSID_SIZE 32
#define PASSWORD_SIZE 64
#define MAX_ATTEMPTS 10

#define GUID_LENGTH 36                // Length of the GUID string
#define GUID_START 100                // EEPROM address to store the GUID
#define GUID_MARKER 0xAA              // Example marker value
#define MARKER_START (GUID_START - 1) // Assuming there's a byte space before GUID_START

const uint8_t AS5600_ADDRESS = 0x36;

Encoder encoder(AS5600_ADDRESS);

SerialNumberManager serialNumberManager(GUID_START, GUID_LENGTH, GUID_MARKER);

const String FIRMWARE_VERSION = "0.1.3";

// Define the motor control pins.
const int rpwmPin = 14; 
const int lpwmPin = 12;
const int renPin = 13;
const int lenPin = 15;

char ssid[SSID_SIZE + 1];
char password[PASSWORD_SIZE + 1];

char serialNumber[37];

bool apMode = false;


EEPROMConfig eepromConfig;
AHT21Sensor aht21Sensor;

// Create an instance of the MotorController class.
MotorController motorController(eepromConfig, aht21Sensor, encoder);

ESP8266WebServer server(80);
APManager apManager("WMC-Config", server, eepromConfig);

ServerManager serverManager(server, motorController, FIRMWARE_VERSION);

void resetWiFiSettings()
{
  WiFi.disconnect(true);
}

void setup()
{
  Serial.begin(115200);
  eepromConfig.begin();
  encoder.begin();
  
  Serial.println();
  Serial.println("Starting WiFi Motor Controller (WMC) Version " + FIRMWARE_VERSION);

  serialNumberManager.begin();
  
  serialNumberManager.readSerialNumber(serialNumber);

  if (!serialNumberManager.isValid())
  {
    // Serial number is not valid. Enter AP mode for configuration.
    Serial.println("Invalid or no Serial Number. Entering AP mode for configuration.");
    apManager.startAPMode();
    apMode = true;
    return; // Stop further execution of setup() to remain in AP mode.
  }

  loadCredentials(ssid, password);
  if (!connectToWifi())
  {
    // Serial number is not valid. Enter AP mode for configuration.
    Serial.println("Invalid or no Serial Number. Entering AP mode for configuration.");
    apManager.startAPMode();
    apMode = true;
    return; // Stop further execution of setup() to remain in AP mode.
  }

  WiFi.hostname("WMC-" + String(serialNumber)); // I want to set the hostname to a name stored in the Eeprom from setup
  if (MDNS.begin("WMC-esp8266"))
  { // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }

  aht21Sensor.begin();

  motorController.init(rpwmPin, lpwmPin, renPin, lenPin);
  // Define routes for commands.
  serverManager.setupEndpoints();
  initializeOTA(); // Initialize OTA
}

void loadCredentials(char *ssid, char *password)
{
  eepromConfig.readSSID(ssid);
  eepromConfig.readPassword(password);
}

bool connectToWifi()
{
  resetWiFiSettings();
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS)
  {
    delay(500 * (attempts + 1)); // Exponential backoff
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed to connect to Wi-Fi. Check your SSID and password.");
    return false;
  }
  Serial.println("Connected to Wi-Fi.");
  return true;
}

void initializeOTA()
{
  ArduinoOTA.onStart([]()
                     {
                       Serial.println("OTA Starting Update");
                       // Add any specific actions you want to perform when OTA starts
                     });

  ArduinoOTA.onEnd([]()
                   {
                     Serial.println("\nOTA Update End");
                     // Actions to perform when OTA ends
                   });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
                          // Update progress
                        });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                         Serial.println("Auth Failed");
                       else if (error == OTA_BEGIN_ERROR)
                         Serial.println("Begin Failed");
                       else if (error == OTA_CONNECT_ERROR)
                         Serial.println("Connect Failed");
                       else if (error == OTA_RECEIVE_ERROR)
                         Serial.println("Receive Failed");
                       else if (error == OTA_END_ERROR)
                         Serial.println("End Failed");
                       // Handle different OTA errors
                     });

  ArduinoOTA.begin();
}

void loop()
{
  if (apMode)
  {
    apManager.handleClient();
  }
  else
  { // TODO: do some timing tests here to see how long it takes to run these command
    encoder.update();
    aht21Sensor.update();
    server.handleClient();
    motorController.update();
    ArduinoOTA.handle(); // Handle OTA
  }
}
