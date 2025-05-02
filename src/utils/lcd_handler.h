#ifndef LCD_HANDLER_H
#define LCD_HANDLER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class LCDHandler {
    LiquidCrystal_I2C* _lcd;
    uint8_t _columns;
    uint8_t _rows;

public:
    LCDHandler(uint8_t lcd_addr, uint8_t columns, uint8_t rows);
    ~LCDHandler();
    
    void initialize() const;
    void clear() const;
    void clearLine(uint8_t line) const;
    void displayTitle(const String& title) const;
    void displayTime(const String& time) const;
    void displayDate(const String& weekday, const String& date) const;
    void displayMsg(const String& message, uint8_t line) const;
    void displayMsgCentered(const String& text, uint8_t line) const;
};

#endif // LCD_HANDLER_H
