#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

#include <Arduino.h>
#include <Ds1302.h>

class RTCHandler {
private:
    Ds1302 rtc;
    
public:
    RTCHandler(uint8_t resetPin, uint8_t clockPin, uint8_t dataPin);
    
    void initialize();
    bool isRunning();
    
    String getFormattedTime();
    String getFormattedDate();
    String getFormattedDateTime();
    
    void setTimeFromEpoch(unsigned long epochSeconds);
    unsigned long getEpochTime();
    
    Ds1302::DateTime getCurrentDateTime();
};

#endif // RTC_HANDLER_H
