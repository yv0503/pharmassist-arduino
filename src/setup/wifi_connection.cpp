#include "wifi_connection.h"

int connectToWiFi(const String& ssid, const String& password, ArduinoLEDMatrix& matrix) {
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  Serial.println();
  
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long previousMillis = 0;
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
      Serial.println("WiFi connection failed");
      break;
    }
  }
  
  if (wifiStatus == WL_CONNECTED) {
    delay(1000L);
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

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
    Serial.println("Failed to connect to WiFi");
    matrix.loadFrame(wifi_matrix[0]);
  }

  return wifiStatus;
}
