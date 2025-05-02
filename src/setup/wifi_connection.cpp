#include <Arduino.h>
#include <WiFiS3.h>
#include <LiquidCrystal_I2C.h>

#include "bluetooth.h"
#include "wifi_connection.h"

#include "constants.h"

extern String lastIpKnown;

int connectToWiFi(const String &ssid, const String &password, const LCDHandler &lcdHandler,
                  const PreferencesHandler &prefsHandler) {
  if (ssid.length() == 0) {
    Serial.println(F("Error: SSID is empty"));
    return WL_CONNECT_FAILED;
  }

  lcdHandler.clear();
  lcdHandler.displayMsgCentered(F("Connecting to WiFi:"), 0);
  lcdHandler.displayMsgCentered(ssid, 1);

  WiFi.disconnect();
  delay(100);

  int status = WL_IDLE_STATUS;
  int attempt = 0;
  constexpr int maxAttempts = 5;

  Serial.print(F("Attempting to connect to SSID: "));
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  while (status != WL_CONNECTED && attempt < maxAttempts) {
    delay(1000);
    status = WiFi.status();

    Serial.print(F("WiFi status: "));
    Serial.println(status);

    attempt++;
    lcdHandler.displayMsgCentered("Attempt " + String(attempt) + "/" + String(maxAttempts), 3);
  }

  if (status == WL_CONNECTED) {
    Serial.println(F("Connected to WiFi"));
    Serial.print(F("IP address: "));
    const IPAddress currentIp = WiFi.localIP();
    Serial.println(currentIp);

    if (!isSameIp(currentIp, IPAddress(lastIpKnown.c_str()))) {
      Serial.print(F("IP address changed from "));
      Serial.print(lastIpKnown);
      Serial.print(F(" to "));
      Serial.println(currentIp);

      // ReSharper disable once CppExpressionWithoutSideEffects, CppNoDiscardExpression
      prefsHandler.setString(PREF_LAST_IP_KNOWN, currentIp.toString());
      broadcastWiFiStatus(status, F("Connected to new IP address."));
    }

    lcdHandler.displayMsgCentered(F("Connected to:"), 0);
    lcdHandler.displayMsgCentered(currentIp.toString(), 2);
    lcdHandler.displayMsgCentered(F("Waiting for app..."), 3);
    delay(500);
  } else {
    Serial.println(F("Failed to connect to WiFi"));
    lcdHandler.displayMsgCentered(F("Connection failed"), 3);
    status = -100;
  }

  return status;
}
