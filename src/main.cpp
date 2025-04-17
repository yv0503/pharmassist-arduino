#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <EEPROM.h>
#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include <constants.h>
#include "setup/wifi_credentials.h"
#include "setup/bluetooth.h"
#include "setup/wifi_connection.h"
#include "networking/web_server.h"

// Arduino UNO R4 WiFi

LiquidCrystal_I2C lcd(0x27, 20, 4);
ArduinoLEDMatrix matrix;
byte isInitialized = 0;

String ssid = "";
String password = "";
unsigned long previousMillis = 0;
constexpr long interval = 1000;
byte currentFrame = 0;
bool wasConnected = false;

const String testServer = "www.google.com";
int wifiStatus = WL_IDLE_STATUS;
int failedAttempts = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
  Serial.println("┃               PharmAssist                 ┃");
  Serial.println("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
  EEPROM.begin();
  matrix.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print("    PharmAssist");

  delay(3000);
  isInitialized = EEPROM.read(isInitializedAddress);

  if (isInitialized != 1) {
    runBluetoothSetup(ssid, password, matrix, lcd);
  }

  loadWiFiCredentials(ssid, password);
  wifiStatus = connectToWiFi(ssid, password, matrix, lcd);

  int retryCount = 0;
  while (wifiStatus != WL_CONNECTED && retryCount < 2) {
    Serial.print("WiFi connection failed. Retry attempt ");
    Serial.print(retryCount + 1);
    Serial.println("/3...");
    retryCount++;
    wifiStatus = connectToWiFi(ssid, password, matrix, lcd);
  }

  // If still failed after 3 attempts, restart Bluetooth setup
  if (wifiStatus != WL_CONNECTED) {
    Serial.println("WiFi connection failed after 3 attempts. Restarting Bluetooth setup...");

    EEPROM.update(isInitializedAddress, 0);

    runBluetoothSetup(ssid, password, matrix, lcd);

    loadWiFiCredentials(ssid, password);
    wifiStatus = connectToWiFi(ssid, password, matrix, lcd);
  }

  if (wifiStatus == WL_CONNECTED) {
    setupWebServer();
    Serial.print("Web server available at http://");
    Serial.println(WiFi.localIP());
  }

  delay(3000);
  matrix.clear();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000L);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    PharmAssist");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    wifiStatus = connectToWiFi(ssid, password, matrix, lcd);
  }

  if (wifiStatus == WL_CONNECTED) {
    handleWebServerClients(lcd);
  }
}

