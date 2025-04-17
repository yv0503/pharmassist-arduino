#include <Arduino.h>

#include "web_server.h"
#include "html_content.h"
#include "api_handler.h"

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
    String method = "";
    String endpoint = "";

    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 2000) {
      if (client.available()) {
        timeout = millis();

        if (const char c = client.read(); c == '\n') {
          if (currentLine.length() == 0) {

            if (endpoint.startsWith("/api/")) {
              auto [statusCode, contentType, body] = ApiHandler::processRequest(endpoint, method, lcd);

              client.print("HTTP/1.1 ");
              client.print(statusCode);
              client.println(" OK");
              client.print("Content-Type: ");
              client.println(contentType);
              client.println("Connection: close");
              client.println();
              client.println(body);
            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.print(HTML_MAIN_CONTENT);
            }
            break;
          }

          if (currentLine.startsWith("GET ") || currentLine.startsWith("POST ")) {
            const int firstSpace = currentLine.indexOf(' ');
            const int secondSpace = currentLine.indexOf(' ', firstSpace + 1);
              
            method = currentLine.substring(0, firstSpace);
            endpoint = currentLine.substring(firstSpace + 1, secondSpace);

            if (const int queryStart = endpoint.indexOf('?'); queryStart != -1) {
              endpoint = endpoint.substring(0, queryStart);
            }
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
  }
}

