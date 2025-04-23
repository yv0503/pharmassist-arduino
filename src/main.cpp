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
#include "utils/rtc_handler.h"
#include "utils/ip_handler.h"

// Arduino UNO R4 WiFi

#define PIN_CLK 5
#define PIN_DAT 4
#define PIN_RST 2
#define LCD_ADDR 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

RTCHandler rtcHandler(PIN_RST, PIN_CLK, PIN_DAT);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLUMNS, LCD_ROWS);
ArduinoLEDMatrix matrix;
byte isInitialized = 0;
bool isDeviceAcknowledged = false;

String ssid = "";
String password = "";
unsigned long previousMillis = 0;
constexpr long interval = 1000;
byte currentFrame = 0;
bool wasConnected = false;

const String testServer = "www.google.com";
int wifiStatus = WL_IDLE_STATUS;
int failedAttempts = 0;

unsigned long lastTimeUpdateMillis = 0;
constexpr unsigned long timeUpdateInterval = 1000;

unsigned long lastWifiCheckMillis = 0;
constexpr unsigned long wifiCheckInterval = 60000;

unsigned long lastBluetoothCheckMillis = 0;
constexpr unsigned long bluetoothCheckInterval = 300000;

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

  rtcHandler.initialize();

  delay(3000);
  isInitialized = EEPROM.read(isInitializedAddress);

  if (isInitialized != 1) {
    runBluetoothSetup(ssid, password, matrix, lcd);
  } else {
    startBluetooth(ssid, password, lcd);
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
    saveLastKnownIp(WiFi.localIP());
    
    broadcastWiFiStatus(wifiStatus, "Connected successfully.", WiFi.localIP().toString(), lcd);
  } else {
    broadcastWiFiStatus(wifiStatus, "Failed to connect.", "0.0.0.0", lcd);
  }

  const unsigned long acknowledgmentTimeoutStart = millis();
  Serial.println("Waiting for device to acknowledge WiFi status (15s timeout)...");
  // ReSharper disable CppDFALoopConditionNotUpdated
  while (!isDeviceAcknowledged && millis() - acknowledgmentTimeoutStart < 15000) {
    // ReSharper restore CppDFALoopConditionNotUpdated
    bluetoothLoop();
    delay(100);
    if (wifiStatus == WL_CONNECTED)
      handleWebServerClients(lcd, rtcHandler, isDeviceAcknowledged);
  }

  if (isDeviceAcknowledged) {
    Serial.println("Device acknowledged WiFi status. Starting Bluetooth timeout.");
  } else {
    Serial.println("No acknowledgment received after timeout. Keeping Bluetooth active.");
  }

  delay(3000);
  matrix.clear();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000L);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    PharmAssist");
}

void loop() {
  const unsigned long currentMillis = millis();

  if (currentMillis - lastWifiCheckMillis >= wifiCheckInterval) {
    lastWifiCheckMillis = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      wifiStatus = connectToWiFi(ssid, password, matrix, lcd);

      if (wifiStatus != WL_CONNECTED && !isBluetoothActive()) {
        startBluetooth(ssid, password, lcd);
      }
    }
  }

  if (currentMillis - lastBluetoothCheckMillis >= bluetoothCheckInterval) {
    lastBluetoothCheckMillis = currentMillis;

    if (!isBleConnected()) {
      restartBleAdvertising(lcd);
    }
  }

  if (wifiStatus == WL_CONNECTED) {
    handleWebServerClients(lcd, rtcHandler, isDeviceAcknowledged);
  }

  if (currentMillis - lastTimeUpdateMillis >= timeUpdateInterval) {
    lastTimeUpdateMillis = currentMillis;

    const String formattedTime = rtcHandler.getFormattedTime();
    const String formattedWeekDay = rtcHandler.getFormattedWeekDay();
    const String formattedDate = rtcHandler.getFormattedDate();

    lcd.setCursor(0, 2);
    lcd.print("  " + formattedWeekDay + ", " + formattedDate);
    lcd.print("    ");

    lcd.setCursor(0, 3);
    lcd.print("      " + formattedTime);
  }

  bluetoothLoop();
}
