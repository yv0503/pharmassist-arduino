#include "wifi_connection.h"

int connectToWiFi(const String &ssid, const String &password, ArduinoLEDMatrix &matrix) {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  Serial.println();

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
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.print("Testing internet connection... ");
    const String testServer = "www.google.com";
    if (WiFiClient client; client.connect(testServer.c_str(), 80)) {
      Serial.println("Success!");
      matrix.loadFrame(wifi_matrix[3]);
      client.stop();
    } else {
      Serial.println("Failed to connect to test server");
      matrix.loadFrame(wifi_matrix[1]);
    }
  } else {
    Serial.print("Failed to connect to WiFi. Status code: ");
    Serial.println(wifiStatus);
    matrix.loadFrame(wifi_matrix[0]);
  }

  return wifiStatus;
}
