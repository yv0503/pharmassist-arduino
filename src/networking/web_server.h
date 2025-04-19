#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include "utils/rtc_handler.h"

void setupWebServer();

void handleWebServerClients(LiquidCrystal_I2C &lcd, RTCHandler &rtcHandler, bool &isDeviceAcknowledged);

#endif
