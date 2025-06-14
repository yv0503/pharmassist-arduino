#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>

#include <constants.h>
#include <WiFi.h>
#include <WiFiTypes.h>

#include "networking/web_server.h"
#include "setup/bluetooth.h"
#include "setup/wifi_connection.h"
#include "utils/rtc_handler.h"
#include "utils/lcd_handler.h"
#include "utils/preferences_handler.h"
#include "utils/servo_handler.h"

/*
 *  Arduino R4 WiFI
 */

// Pin definitions
#define PIN_CLK 9
#define PIN_DAT 8
#define PIN_RST 7
#define PIN_SPI_CS 4
#define PIN_RED_BUTTON 2
#define PIN_BLUE_BUTTON 3
#define PIN_SERVO 5

#define LCD_ADDR 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

// Device state variables
String ssid = "";
String password = "";
String lastIpKnown = "";
int wifiStatus = WL_IDLE_STATUS;
bool isInitialized = false;
bool isDeviceAcknowledged = false;
volatile bool redButtonFlag = false;
volatile bool blueButtonFlag = false;

// Timing variables
unsigned long lastTimeUpdateMillis = 0;
unsigned long lastWifiCheckMillis = 0;
unsigned long lastBluetoothCheckMillis = 0;

// Time intervals (in milliseconds)
constexpr unsigned long TIME_UPDATE_INTERVAL = 1000;
constexpr unsigned long ACKNOWLEDGEMENT_TIMEOUT = 30000;
constexpr unsigned long WIFI_CHECK_INTERVAL = 60000;
constexpr unsigned long BLUETOOTH_CHECK_INTERVAL = 10000;
constexpr unsigned long STARTUP_DELAY = 2000;
constexpr unsigned long SERVO_TURN_TIME = 200;

//Container test variables
unsigned int CURRENT_CONTAINER = 0;
unsigned int NEXT_CONTAINER = 2;

RTCHandler rtcHandler(PIN_RST, PIN_CLK, PIN_DAT);
LCDHandler lcdHandler(LCD_ADDR, LCD_COLUMNS, LCD_ROWS);
ServoHandler servoHandler;
PreferencesHandler prefsHandler;

void redButtonPressed() { redButtonFlag = true; }
void blueButtonPressed() { blueButtonFlag = true; }

void setup() {
    Serial.begin(9600);
    Serial.println(F("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓"));
    Serial.println(F("┃               PharmAssist                 ┃"));
    Serial.println(F("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n"));

    lcdHandler.initialize();
    lcdHandler.displayTitle(DEVICE_NAME);
    servoHandler.initialize(PIN_SERVO);

    Serial.println(F("Starting..."));
    delay(STARTUP_DELAY);

    rtcHandler.initialize();
    Serial.println("Time: " + rtcHandler.getFormattedDateTime());

    pinMode(PIN_RED_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BLUE_BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_RED_BUTTON), redButtonPressed, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BLUE_BUTTON), blueButtonPressed, FALLING);

    if (!SD.begin(PIN_SPI_CS)) {
        Serial.println(F("µSD card not found"));
        lcdHandler.clear();
        lcdHandler.displayMsgCentered(F("SD card not"), 0);
        lcdHandler.displayMsgCentered(F("found or failed"), 1);
        lcdHandler.displayMsgCentered(F("Restart your device"), 2);
        // ReSharper disable once CppDFAEndlessLoop
        while (true);
    }

    Serial.println(F("µSD card found"));

    if (!prefsHandler.preferencesExist()) runBluetoothSetup(ssid, password, prefsHandler, lcdHandler);
    if (!isBleActive()) startBluetooth(ssid, password, lcdHandler, prefsHandler);

    ssid = prefsHandler.getString(PREF_WIFI_SSID);
    password = prefsHandler.getString(PREF_WIFI_PASSWORD);
    lastIpKnown = prefsHandler.getString(PREF_LAST_IP_KNOWN);
    wifiStatus = connectToWiFi(ssid, password, lcdHandler, prefsHandler);

    if (wifiStatus == -100) {
        runBluetoothSetup(ssid, password, prefsHandler, lcdHandler);
        wifiStatus = connectToWiFi(ssid, password, lcdHandler, prefsHandler);
    }

    if (wifiStatus == WL_CONNECTED) {
        setupWebServer();
        Serial.print(F("Web server available at http://"));
        const IPAddress localIp = WiFi.localIP();
        Serial.println(localIp);
        broadcastWiFiStatus(wifiStatus, F("Connected successfully."));
    } else {
        broadcastWiFiStatus(wifiStatus, F("Failed to connect."));
    }

    const unsigned long acknowledgmentTimeoutStart = millis();
    Serial.println(F("Waiting for device to acknowledge WiFi status (30s timeout)..."));

    while (!isDeviceAcknowledged && millis() - acknowledgmentTimeoutStart < ACKNOWLEDGEMENT_TIMEOUT) {
        bluetoothLoop();
        delay(100);
        if (wifiStatus == WL_CONNECTED) {
            handleWebServerClients(prefsHandler, lcdHandler, rtcHandler, isDeviceAcknowledged);
        }
    }

    if (isDeviceAcknowledged) {
        Serial.println(F("Device acknowledged WiFi status. Starting Bluetooth timeout."));
        resetBluetoothTimeout();
    } else {
        Serial.println(F("No acknowledgment received after timeout. Keeping Bluetooth active."));
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(STARTUP_DELAY);
    lcdHandler.clear();
    lcdHandler.displayMsgCentered(DEVICE_NAME.c_str(), 1);
}

void loop() {
    const unsigned long currentMillis = millis();

    if (currentMillis - lastTimeUpdateMillis >= TIME_UPDATE_INTERVAL) {
        lastTimeUpdateMillis = currentMillis;

        const String formattedTime = rtcHandler.getFormattedTime();
        const String formattedWeekDay = rtcHandler.getFormattedWeekDay();
        const String formattedDate = rtcHandler.getFormattedDate();

        lcdHandler.displayTitle("PharmAssist");
        lcdHandler.clearLine(1);
        lcdHandler.displayDate(formattedWeekDay, formattedDate);
        lcdHandler.displayTime(formattedTime);
    }

    if (currentMillis - lastBluetoothCheckMillis >= BLUETOOTH_CHECK_INTERVAL) {
        lastBluetoothCheckMillis = currentMillis;

        if (isBluetoothTimeoutElapsed() && isDeviceAcknowledged) {
            Serial.println(F("Bluetooth timeout reached. Stopping Bluetooth."));
            stopBluetooth();
        }
    }

    if (currentMillis - lastWifiCheckMillis >= WIFI_CHECK_INTERVAL) {
        lastWifiCheckMillis = currentMillis;

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println(F("WiFi connection lost. Attempting to reconnect..."));
            lcdHandler.displayMsgCentered(F("WiFi connection lost."), 1);
            lcdHandler.displayMsgCentered(F("Reconnecting..."), 2);
            wifiStatus = connectToWiFi(ssid, password, lcdHandler, prefsHandler);

            if (wifiStatus != WL_CONNECTED) {
                isDeviceAcknowledged = false;
                isInitialized = false;
                runBluetoothSetup(ssid, password, prefsHandler, lcdHandler);
            }
        }
    }

    if (wifiStatus == WL_CONNECTED) {
        handleWebServerClients(prefsHandler, lcdHandler, rtcHandler, isDeviceAcknowledged);
    }

    if (isBleActive()) {
        bluetoothLoop();
    }

    if (redButtonFlag) {
        redButtonFlag = false;
        Serial.println(F("Red button pressed"));
        lcdHandler.clear();
        lcdHandler.displayMsgCentered(F("Red button pressed"), 1);
        servoHandler.toMedicineContainer(SERVO_TURN_TIME, CURRENT_CONTAINER, NEXT_CONTAINER);
        delay(2000);
        lcdHandler.clear();
    }

    if (blueButtonFlag) {
        blueButtonFlag = false;
        Serial.println(F("Blue button pressed - Restarting Bluetooth"));
        lcdHandler.clear();
        lcdHandler.displayMsgCentered(F("Bluetooth Restarted"), 1);
        servoHandler.resetPosition(SERVO_TURN_TIME, CURRENT_CONTAINER);
        startBluetooth(ssid, password, lcdHandler, prefsHandler);
        delay(2000);
        lcdHandler.clear();
    }
}
