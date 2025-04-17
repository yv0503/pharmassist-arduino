#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

// Initialize the web server
void setupWebServer();

// Handle client requests
void handleWebServerClients(LiquidCrystal_I2C &lcd);

#endif
