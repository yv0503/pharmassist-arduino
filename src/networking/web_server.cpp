#include <Arduino.h>

#include "web_server.h"
#include "html_content.h"
#include "api_handler.h"

WiFiServer server(80);

void setupWebServer() {
    server.begin();
    Serial.println(F("Web server started"));
}

void handleWebServerClients(
    const PreferencesHandler &prefsHandler,
    LCDHandler &lcdHandler,
    RTCHandler &rtcHandler,
    bool &isDeviceAcknowledged
) {
    if (WiFiClient client = server.available()) {
        Serial.print(F("New client connected: "));
        Serial.println(client.remoteIP());

        String currentLine = "";
        String method = "";
        String endpoint = "";
        String requestBody = "";

        int contentLength = 0;

        unsigned long timeout = millis();
        while (client.connected() && millis() - timeout < 2000) {
            if (client.available()) {
                timeout = millis();

                if (const char c = static_cast<char>(client.read()); c == '\n') {
                    if (currentLine.length() == 0) {
                        if (contentLength > 0 && (method == F("POST") || method == F("PUT"))) {
                            int bodyBytesRead = 0;
                            while (bodyBytesRead < contentLength && client.available() && millis() - timeout < 2000) {
                                const char ch = static_cast<char>(client.read());
                                requestBody += ch;
                                bodyBytesRead++;
                                timeout = millis();
                            }
                            Serial.print(F("Request body: "));
                            Serial.println(requestBody);
                        }

                        if (endpoint.startsWith(F("/api/"))) {
                            auto [statusCode, contentType, body] =
                                    ApiHandler::processRequest(endpoint, method, requestBody, prefsHandler,
                                                               isDeviceAcknowledged, lcdHandler, rtcHandler);

                            client.print(F("HTTP/1.1 "));
                            client.print(statusCode);
                            client.println(F(" OK"));
                            client.print(F("Content-Type: "));
                            client.println(contentType);
                            client.println(F("Connection: close"));
                            client.println();
                            client.println(body);
                        } else {
                            client.println(F("HTTP/1.1 200 OK"));
                            client.println(F("Content-type:text/html"));
                            client.println(F("Connection: close"));
                            client.println();
                            client.print(HTML_MAIN_CONTENT);
                        }
                        break;
                    }

                    if (currentLine.startsWith(F("GET ")) || currentLine.startsWith(F("POST "))) {
                        const int firstSpace = currentLine.indexOf(' ');
                        const int secondSpace = currentLine.indexOf(' ', firstSpace + 1);

                        method = currentLine.substring(0, firstSpace);
                        endpoint = currentLine.substring(firstSpace + 1, secondSpace);

                        if (const int queryStart = endpoint.indexOf('?'); queryStart != -1) {
                            endpoint = endpoint.substring(0, queryStart);
                        }
                    } else if (currentLine.startsWith(F("Content-Length: "))) {
                        contentLength = currentLine.substring(16).toInt();
                        Serial.print(F("Content Length: "));
                        Serial.println(contentLength);
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
