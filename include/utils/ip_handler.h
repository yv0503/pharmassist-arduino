#ifndef IP_HANDLER_H
#define IP_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <constants.h>

inline void saveLastKnownIp(IPAddress ip) {
  for (int i = 0; i < 4; i++) {
    EEPROM.update(lastKnownIpStartAddress + i, ip[i]);
  }
}

inline IPAddress loadLastKnownIp() {
  byte ipBytes[4];
  for (int i = 0; i < 4; i++) {
    ipBytes[i] = EEPROM.read(lastKnownIpStartAddress + i);
  }
  
  return {ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]};
}

inline bool isSameIp(IPAddress ip1, IPAddress ip2) {
  for (int i = 0; i < 4; i++) {
    if (ip1[i] != ip2[i]) {
      return false;
    }
  }
  return true;
}

#endif // IP_HANDLER_H
