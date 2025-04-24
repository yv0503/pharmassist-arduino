#include <Arduino.h>
#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <LiquidCrystal_I2C.h>

#include <utils/ip_handler.h>
#include "../led_matrix/wifi_matrix.h"
#include "bluetooth.h"
#include "wifi_connection.h"

int connectToWiFi(const String &ssid, const String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd) {
  if (ssid.length() == 0) {
    Serial.println(F("Error: SSID is empty"));
    return WL_CONNECT_FAILED;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("    Connecting to"));
  lcd.setCursor(0, 1);
  lcd.print(F("       Wi-Fi"));
  lcd.setCursor(0, 2);
  lcd.print("      " + ssid);

  matrix.begin();
  matrix.loadFrame(wifi_matrix[0]);

  WiFi.disconnect();
  delay(100);
  
  int status = WL_IDLE_STATUS;
  int attempt = 0;
  constexpr int maxAttempts = 10;
  byte currentFrame = 0;
  
  Serial.print(F("Attempting to connect to SSID: "));
  Serial.println(ssid);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (status != WL_CONNECTED && attempt < maxAttempts) {
    delay(1000);
    status = WiFi.status();
    Serial.print(F("WiFi status: "));
    Serial.println(status);
    currentFrame = 1 - currentFrame;
    matrix.loadFrame(wifi_matrix[currentFrame]);
    attempt++;
    lcd.setCursor(0, 3);
    lcd.print("   Attempt " + String(attempt) + "/" + String(maxAttempts));
  }
  
  if (status == WL_CONNECTED) {
    Serial.println(F("Connected to WiFi"));
    Serial.print(F("IP address: "));
    const IPAddress currentIp = WiFi.localIP();
    Serial.println(currentIp);

    const IPAddress lastKnownIp = loadLastKnownIp();
    if (!isSameIp(currentIp, lastKnownIp)) {
      Serial.print(F("IP address changed from "));
      Serial.print(lastKnownIp);
      Serial.print(F(" to "));
      Serial.println(currentIp);

      saveLastKnownIp(currentIp);
      broadcastWiFiStatus(status, F("Connected to new IP address."), lcd);
    }
    
    matrix.loadFrame(wifi_matrix[1]);
    lcd.setCursor(0, 3);
    lcd.print(F("      Connected     "));
    delay(500);
  } else {
    Serial.println(F("Failed to connect to WiFi"));
    lcd.setCursor(0, 3);
    lcd.print(F("  Connection failed  "));
  }
  
  return status;
}
