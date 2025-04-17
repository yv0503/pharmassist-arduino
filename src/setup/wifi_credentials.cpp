#include <EEPROM.h>

#include <constants.h>
#include "wifi_credentials.h"

void saveWiFiCredentials(const String &ssid, const String &password) {
    EEPROM.update(isInitializedAddress, 1);

    for (int i = 0; i < ssid.length(); i++) {
        EEPROM.update(ssidStartAddress + i, ssid[i]);
    }
    EEPROM.update(ssidStartAddress + ssid.length(), 0);

    for (int i = 0; i < password.length(); i++) {
        EEPROM.update(passwordStartAddress + i, password[i]);
    }
    EEPROM.update(passwordStartAddress + password.length(), 0);

    Serial.println("WiFi credentials saved to EEPROM");
}

void loadWiFiCredentials(String &ssid, String &password) {
    ssid = "";
    for (int i = 0; i < maxStrLength; i++) {
        const char c = EEPROM.read(ssidStartAddress + i);
        if (c == 0) break;
        ssid += c;
    }

    password = "";
    for (int i = 0; i < maxStrLength; i++) {
        const char c = EEPROM.read(passwordStartAddress + i);
        if (c == 0) break;
        password += c;
    }

    Serial.println("Loaded SSID: " + ssid);
    Serial.println("Loaded password: " + String(password.length()) + " characters");
}
