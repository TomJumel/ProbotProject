//
// Created by Tom on 27/02/2021.
//

#ifndef PROGRAMM_ROBOTMOVE_H
#define PROGRAMM_ROBOTMOVE_H

#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32PWM.h>
#else

#include <Servo.h>

#endif

#include <Arduino.h>

enum _robotHbridgeType {
    TWO_MOTORS, SINGLE_MOTOR
};
enum _robotState {
    IDLE, FORWARD, BACKWARD
};
enum _directionMode{
    SERVO,HBRIDGE
};
class RobotMove {
public:
    RobotMove(uint8_t motor_N1,
              uint8_t motor_N2,
              uint8_t motor_N3,
              uint8_t motor_N4,
              uint8_t speedPin);

    RobotMove(uint8_t motor_N1,
              uint8_t motor_N2,
              uint8_t speedPin);

    void init();
    void setMinPWM(uint8_t newPWMMin);

    void forward(uint8_t speedPercent);

    void backward(uint8_t speedPercent);

    void slideToSpeed(uint8_t speedPercent);

    void bridle(uint8_t newPWMMax);

    void brake();

    void turnRight(uint8_t anglePercent);

    void turnLeft(uint8_t anglePercent);

    void parseRequest(String str);

private:
    uint8_t _motor_N1;
    uint8_t _motor_N2;
    uint8_t _motor_N3;
    uint8_t _motor_N4;
    _robotHbridgeType robotHbridgeType;
    _robotState robotState = IDLE;
    _directionMode dirmode = SERVO;
    uint8_t _speedPin;
    uint8_t _PwmDirectionPin;
    uint8_t _calibServo = 90;
    uint8_t _PWMMin = 0;
    uint8_t _PWMMax = 255;
    uint8_t _cSpeed = 0;
    Servo _servo;

    uint8_t _getAngleValue(uint8_t value, bool right);

    uint8_t _getPWMValue(uint8_t value);

    void setSpeed(uint8_t speedPercent);

    void setState(_robotState robotState);
    uint8_t _getRotationPWMValue(uint8_t value);
};


#endif //PROGRAMM_ROBOTMOVE_H
