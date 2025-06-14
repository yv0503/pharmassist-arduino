#ifndef SERVO_HANDLER_H
#define SERVO_HANDLER_H

#include <Arduino.h>
#include <Servo.h>

class ServoHandler {
    Servo* _servo;

public:
    ServoHandler();

    void initialize(uint8_t servo_pin)const;
    void toNextContainer(uint8_t timeInMillis, uint8_t currentContainer, uint8_t nextContainer) const;
    void resetPosition(uint8_t timeInMillis, uint8_t currentContainer) const;
};

#endif //SERVO_HANDLER_H
