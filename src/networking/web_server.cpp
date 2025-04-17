#include <Arduino.h>
#include <EEPROM.h>

#include <constants.h>
#include "web_server.h"
#include "html_content.h"

WiFiServer server(80);

void setupWebServer() {
  server.begin();
  Serial.println("Web server started");
}

void handleWebServerClients(LiquidCrystal_I2C &lcd) {
  if (WiFiClient client = server.available()) {
    Serial.print("New client connected: ");
    Serial.println(client.remoteIP());
    String currentLine = "";
    bool isResetRequested = false;

    while (client.connected()) {
      if (client.available()) {
        if (const char c = client.read(); c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            if (isResetRequested) {
              EEPROM.update(isInitializedAddress, 0);
              digitalWrite(LED_BUILTIN, LOW);
              Serial.println("Factory reset performed");
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print(" Factory reset done");
              lcd.setCursor(0, 2);
              lcd.print("  Restart Required");

              client.println(HTML_RESET_CONTENT);

              delay(2000);
            } else {
              client.print(HTML_MAIN_CONTENT);
            }

            client.println();
            break;
          }

          if (currentLine.startsWith("GET /reset")) {
            isResetRequested = true;
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
