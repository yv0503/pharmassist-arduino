#include "wifi_connection.h"

int connectToWiFi(const String &ssid, const String &password, ArduinoLEDMatrix &matrix, LiquidCrystal_I2C &lcd) {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long previousMillis = 0;
  const unsigned long connectionStartTime = millis();
  byte currentFrame = 0;
  int wifiStatus = WL_IDLE_STATUS;

  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.status();

    if (const unsigned long currentMillis = millis(); currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      matrix.loadFrame(wifi_matrix[currentFrame]);
      currentFrame = (currentFrame + 1) % 4;

      lcd.setCursor(0, 2);
      lcd.print("Connecting");
      for (int i = 0; i < currentFrame + 1; i++) {
        lcd.print(".");
      }
      lcd.print("   ");
    }

    if (wifiStatus == WL_CONNECT_FAILED) {
      Serial.println("WiFi connection failed - invalid credentials or network issues");
      break;
    }

    if (constexpr unsigned long CONNECTION_TIMEOUT = 5000; millis() - connectionStartTime > CONNECTION_TIMEOUT) {
      Serial.println("WiFi connection timeout");
      wifiStatus = WL_CONNECTION_LOST;
      break;
    }
  }

  if (wifiStatus == WL_CONNECTED) {
    delay(1000L);
    Serial.println("Connected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Wi-Fi Connected!  ");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());
    lcd.setCursor(0, 2);
    lcd.print("RSSI: ");
    lcd.print(WiFi.RSSI());
    lcd.print(" dBm");

    Serial.print("Testing internet connection... ");
    const String testServer = "www.google.com";
    if (WiFiClient client; client.connect(testServer.c_str(), 80)) {
      Serial.println("Success!");
      matrix.loadFrame(wifi_matrix[3]);
      lcd.setCursor(0, 3);
      lcd.print("Internet: Connected");
      client.stop();
    } else {
      Serial.println("Failed to connect to test server");
      matrix.loadFrame(wifi_matrix[1]);
      lcd.setCursor(0, 3);
      lcd.print("Internet: Failed");
    }
  } else {
    Serial.print("Failed to connect to WiFi. Status code: ");
    Serial.println(wifiStatus);
    matrix.loadFrame(wifi_matrix[0]);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connection");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");
    lcd.setCursor(0, 2);
    lcd.print("Status code: ");
    lcd.print(wifiStatus);
  }

  return wifiStatus;
}
