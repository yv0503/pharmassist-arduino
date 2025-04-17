#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

void setupWebServer();

void handleWebServerClients(LiquidCrystal_I2C &lcd);

#endif
