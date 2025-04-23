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
    Serial.println("Error: SSID is empty");
    return WL_CONNECT_FAILED;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("       Wi-Fi");
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
  
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (status != WL_CONNECTED && attempt < maxAttempts) {
    delay(1000);
    status = WiFi.status();
    Serial.print("WiFi status: ");
    Serial.println(status);
    currentFrame = 1 - currentFrame;
    matrix.loadFrame(wifi_matrix[currentFrame]);
    attempt++;
    lcd.setCursor(0, 3);
    lcd.print("   Attempt " + String(attempt) + "/" + String(maxAttempts));
  }
  
  if (status == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    const IPAddress currentIp = WiFi.localIP();
    Serial.println(currentIp);

    if (const IPAddress lastKnownIp = loadLastKnownIp(); !isSameIp(currentIp, lastKnownIp)) {
      Serial.print("IP address changed from ");
      Serial.print(lastKnownIp);
      Serial.print(" to ");
      Serial.println(currentIp);

      saveLastKnownIp(currentIp);
      broadcastIpChange(currentIp.toString());
    }
    
    matrix.loadFrame(wifi_matrix[1]);
    lcd.setCursor(0, 3);
    lcd.print("      Connected     ");
    delay(500);
  } else {
    Serial.println("Failed to connect to WiFi");
    lcd.setCursor(0, 3);
    lcd.print("  Connection failed  ");
  }
  
  return status;
}
