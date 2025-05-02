#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "utils/lcd_handler.h"
#include "utils/preferences_handler.h"
#include "utils/rtc_handler.h"

void setupWebServer();

void handleWebServerClients(const PreferencesHandler &prefsHandler, LCDHandler &lcdHandler, RTCHandler &rtcHandler, bool &isDeviceAcknowledged);

#endif
