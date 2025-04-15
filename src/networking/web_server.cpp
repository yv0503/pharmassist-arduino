#include "web_server.h"
#include "html_content.h"
#include "../constants.h"
#include <Arduino.h>
#include <EEPROM.h>

WiFiServer server(80);

void setupWebServer() {
  server.begin();
  Serial.println("Web server started");
}

void handleWebServerClients() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.print("New client connected: ");
    Serial.println(client.remoteIP());
    String currentLine = "";
    bool isResetRequested = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            if (isResetRequested) {
              EEPROM.write(isInitializedAddress, 0);
              digitalWrite(LED_BUILTIN, LOW);
              Serial.println("Factory reset performed");

              client.println(HTML_RESET_CONTENT);

              delay(2000);
            } else {
              client.print(HTML_MAIN_CONTENT);
            }

            client.println();
            break;
          } else {
            if (currentLine.startsWith("GET /reset")) {
              isResetRequested = true;
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
