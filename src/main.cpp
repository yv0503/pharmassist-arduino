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

// Pin definitions
#define PIN_CLK 5
#define PIN_DAT 4
#define PIN_RST 2
#define LCD_ADDR 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

RTCHandler rtcHandler(PIN_RST, PIN_CLK, PIN_DAT);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLUMNS, LCD_ROWS);
ArduinoLEDMatrix matrix;

// Device state variables
byte isInitialized = 0;
bool isDeviceAcknowledged = false;
String ssid = "";
String password = "";
int wifiStatus = WL_IDLE_STATUS;

// Timing variables
unsigned long lastTimeUpdateMillis = 0;
unsigned long lastWifiCheckMillis = 0;
unsigned long lastBluetoothCheckMillis = 0;

// Time intervals (in milliseconds)
constexpr unsigned long TIME_UPDATE_INTERVAL = 1000;
constexpr unsigned long WIFI_CHECK_INTERVAL = 60000;
constexpr unsigned long BLUETOOTH_CHECK_INTERVAL = 300000;
constexpr unsigned long ACKNOWLEDGEMENT_TIMEOUT = 30000;
constexpr unsigned long STARTUP_DELAY = 3000;

void setup() {
  Serial.begin(9600);
  Serial.println(F("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓"));
  Serial.println(F("┃               PharmAssist                 ┃"));
  Serial.println(F("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n"));
  
  // Initialize hardware components
  EEPROM.begin();
  matrix.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print(F("    PharmAssist"));

  rtcHandler.initialize();
  delay(STARTUP_DELAY);
  
  // Check initialization status
  isInitialized = EEPROM.read(isInitializedAddress);

  // Handle Bluetooth setup or reconnect based on initialization status
  if (isInitialized != 1) {
    runBluetoothSetup(ssid, password, matrix, lcd);
  } else {
    startBluetooth(ssid, password, lcd);
  }

  // Connect to Wi-Fi with a retry mechanism
  loadWiFiCredentials(ssid, password);
  wifiStatus = connectToWiFi(ssid, password, matrix, lcd);

  // Retry Wi-Fi connection up to 2 more times
  for (int retryCount = 0; retryCount < 2 && wifiStatus != WL_CONNECTED; retryCount++) {
    Serial.print(F("WiFi connection failed. Retry attempt "));
    Serial.print(retryCount + 1);
    Serial.println(F("/3..."));
    wifiStatus = connectToWiFi(ssid, password, matrix, lcd);
  }

  // If the Wi-Fi connection still fails, reset and restart Bluetooth setup
  if (wifiStatus != WL_CONNECTED) {
    Serial.println(F("WiFi connection failed after 3 attempts. Restarting Bluetooth setup..."));
    EEPROM.update(isInitializedAddress, 0);
    runBluetoothSetup(ssid, password, matrix, lcd);
    loadWiFiCredentials(ssid, password);
    wifiStatus = connectToWiFi(ssid, password, matrix, lcd);
  }

  // Setup web server if connected successfully
  if (wifiStatus == WL_CONNECTED) {
    setupWebServer();
    Serial.print(F("Web server available at http://"));
    Serial.println(WiFi.localIP());
    saveLastKnownIp(WiFi.localIP());
    broadcastWiFiStatus(wifiStatus, F("Connected successfully."), lcd);
  } else {
    broadcastWiFiStatus(wifiStatus, F("Failed to connect."), lcd);
  }

  // Wait for device acknowledgment with timeout
  const unsigned long acknowledgmentTimeoutStart = millis();
  Serial.println(F("Waiting for device to acknowledge WiFi status (35s timeout)..."));
  
  while (!isDeviceAcknowledged && millis() - acknowledgmentTimeoutStart < ACKNOWLEDGEMENT_TIMEOUT) {
    bluetoothLoop();
    delay(100);
    if (wifiStatus == WL_CONNECTED) {
      handleWebServerClients(lcd, rtcHandler, isDeviceAcknowledged);
    }
  }

  if (isDeviceAcknowledged) {
    Serial.println(F("Device acknowledged WiFi status. Starting Bluetooth timeout."));
  } else {
    Serial.println(F("No acknowledgment received after timeout. Keeping Bluetooth active."));
  }

  // Final setup display
  delay(STARTUP_DELAY);
  matrix.clear();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(STARTUP_DELAY);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("    PharmAssist"));
}

void loop() {
  const unsigned long currentMillis = millis();

  // Periodic Wi-Fi connection check
  if (currentMillis - lastWifiCheckMillis >= WIFI_CHECK_INTERVAL) {
    lastWifiCheckMillis = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("WiFi connection lost. Attempting to reconnect..."));
      wifiStatus = connectToWiFi(ssid, password, matrix, lcd);

      // Start Bluetooth if Wi-Fi fails and Bluetooth isn't active
      if (wifiStatus != WL_CONNECTED && !isBluetoothActive()) {
        startBluetooth(ssid, password, lcd);
      }
    }
  }

  // Periodic Bluetooth connection check
  if (currentMillis - lastBluetoothCheckMillis >= BLUETOOTH_CHECK_INTERVAL) {
    lastBluetoothCheckMillis = currentMillis;

    if (!isBleConnected()) {
      restartBleAdvertising(lcd);
    }
  }

  // Handle web server clients if Wi-Fi is connected
  if (wifiStatus == WL_CONNECTED) {
    handleWebServerClients(lcd, rtcHandler, isDeviceAcknowledged);
  }

  // Update time display
  if (currentMillis - lastTimeUpdateMillis >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdateMillis = currentMillis;

    const String formattedTime = rtcHandler.getFormattedTime();
    const String formattedWeekDay = rtcHandler.getFormattedWeekDay();
    const String formattedDate = rtcHandler.getFormattedDate();

    lcd.setCursor(0, 2);
    lcd.print("  " + formattedWeekDay + ", " + formattedDate + "    ");

    lcd.setCursor(0, 3);
    lcd.print("      " + formattedTime);
  }

  // Process Bluetooth events
  bluetoothLoop();
}
