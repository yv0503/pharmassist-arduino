#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

class AlarmHandler {

public:

    void initialize(uint8_t alarm_pin);
    void playAlarm(uint8_t alarm_pin) const;
    void stopAlarm(uint8_t alarm_pin) const;
};

#endif //ALARM_H
