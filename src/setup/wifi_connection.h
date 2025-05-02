#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>

#include "utils/preferences_handler.h"


inline bool isSameIp(IPAddress ip1, IPAddress ip2) {
    for (int i = 0; i < 4; i++) {
        if (ip1[i] != ip2[i]) {
            return false;
        }
    }
    return true;
}

int connectToWiFi(const String &ssid, const String &password, const LCDHandler &lcdHandler,
                  const PreferencesHandler &prefsHandler);

#endif // WIFI_CONNECTION_H
