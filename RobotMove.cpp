#include "RobotMove.h"
#include "Probot.h"
RobotMove::RobotMove(uint8_t motor_N1, uint8_t motor_N2,
                     uint8_t motor_N3, uint8_t motor_N4,
                     uint8_t speedPin)
{
    _motor_N1 = motor_N1;
    _motor_N2 = motor_N2;
    _motor_N3 = motor_N3;
    _motor_N4 = motor_N4;
    _speedPin = speedPin;
    robotHbridgeType = TWO_MOTORS;
}

RobotMove::RobotMove(uint8_t motor_N1, uint8_t motor_N2,
                     uint8_t speedPin)
{
    _motor_N1 = motor_N1;
    _motor_N2 = motor_N2;
    _speedPin = speedPin;
    robotHbridgeType = SINGLE_MOTOR;
}

void RobotMove::init()
{
#ifdef USE_SERVO_DIRECTION
    _servo.attach(_PwmDirectionPin);
#elif defined(USE_HBRIDGE_DIRECTION)
    ledcAttachPin(HBRDIGE_SPEED_DIRECTION, 14);
    ledcSetup(14, 20000, 8);
    pinMode(HBRDIGE_LEFT_DIRECTION, OUTPUT);
    pinMode(HBRDIGE_RIGHT_DIRECTION, OUTPUT);
#endif
    pinMode(_motor_N1, OUTPUT);
    pinMode(_motor_N2, OUTPUT);
    pinMode(_motor_N3, OUTPUT);
    pinMode(_motor_N4, OUTPUT);
    pinMode(_speedPin, OUTPUT);
    ledcAttachPin(_speedPin, 15);
    ledcSetup(15, 20000, 8);
}

void RobotMove::setMinPWM(uint8_t newPWMMin)
{
    _PWMMin = newPWMMin;
}

void RobotMove::bridle(uint8_t newPWMMax)
{
    _PWMMax = newPWMMax;
}

void RobotMove::forward(uint8_t speedPercent)
{
    this->setState(FORWARD);
    this->setSpeed(speedPercent);
}

void RobotMove::backward(uint8_t speedPercent)
{
    this->setState(BACKWARD);
    this->setSpeed(speedPercent);
}

void RobotMove::setSpeed(uint8_t speedPercent)
{
    ledcWrite(15, _getPWMValue(speedPercent));
}

void RobotMove::setState(_robotState robotState)
{
    this->robotState = robotState;
    switch (this->robotState)
    {
    case IDLE:
        if (robotHbridgeType == TWO_MOTORS)
        {
            digitalWrite(_motor_N1, LOW);
            digitalWrite(_motor_N2, LOW);
            digitalWrite(_motor_N3, LOW);
            digitalWrite(_motor_N4, LOW);
        }
        if (robotHbridgeType == SINGLE_MOTOR)
        {
            digitalWrite(_motor_N1, LOW);
            digitalWrite(_motor_N2, LOW);
        }
        break;
    case FORWARD:
        if (robotHbridgeType == TWO_MOTORS)
        {
            digitalWrite(_motor_N1, HIGH);
            digitalWrite(_motor_N2, LOW);
            digitalWrite(_motor_N3, HIGH);
            digitalWrite(_motor_N4, LOW);
        }
        if (robotHbridgeType == SINGLE_MOTOR)
        {
            digitalWrite(_motor_N1, HIGH);
            digitalWrite(_motor_N2, LOW);
        }
        break;
    case BACKWARD:
        if (robotHbridgeType == TWO_MOTORS)
        {
            digitalWrite(_motor_N1, LOW);
            digitalWrite(_motor_N2, HIGH);
            digitalWrite(_motor_N3, LOW);
            digitalWrite(_motor_N4, HIGH);
        }
        if (robotHbridgeType == SINGLE_MOTOR)
        {
            digitalWrite(_motor_N1, LOW);
            digitalWrite(_motor_N2, HIGH);
        }
        break;
    }
}

void RobotMove::brake()
{
    int delta = _cSpeed - _PWMMin;
    int waitTime = 1000 / delta;
    if (delta > 100)
        waitTime = 2000 / delta;
    for (int brake = _cSpeed; brake >= _PWMMin; brake--)
    {
        setSpeed(brake);
        delay(waitTime);
    }
    setState(IDLE);
}

void RobotMove::turnRight(uint8_t anglePercent)
{
#ifdef USE_SERVO_DIRECTION
    _servo.write(_getAngleValue(anglePercent, true));
#elif defined(USE_HBRIDGE_DIRECTION)
    ledcWrite(14, _getRotationPWMValue(anglePercent));
    digitalWrite(HBRDIGE_LEFT_DIRECTION, LOW);
    digitalWrite(HBRDIGE_RIGHT_DIRECTION, anglePercent!=0?HIGH:LOW);
#endif
}

void RobotMove::turnLeft(uint8_t anglePercent)
{
#ifdef USE_SERVO_DIRECTION
    _servo.write(_getAngleValue(anglePercent, false));
#elif defined(USE_HBRIDGE_DIRECTION)
    ledcWrite(14, _getRotationPWMValue(anglePercent));
    digitalWrite(HBRDIGE_LEFT_DIRECTION,anglePercent!=0?HIGH:LOW);
    digitalWrite(HBRDIGE_RIGHT_DIRECTION, LOW);
#endif
}

uint8_t RobotMove::_getAngleValue(uint8_t value, bool right)
{
    return map(value, 0, 100, _calibServo, right ? 180 : 0);
}

uint8_t RobotMove::_getPWMValue(uint8_t value)
{
    return map(value, 0, 255, _PWMMin, _PWMMax);
}
uint8_t RobotMove::_getRotationPWMValue(uint8_t value)
{
    return map(value, 0, 255, HBRIDGE_MOTOR_MIN, 255);
}
void RobotMove::parseRequest(String str)
{
    switch (str[0])
    {
    case 'F':
        forward(str[1]);
        break;
    case 'B':
        backward(str[1]);
        break;
    case 'L':
        turnLeft(str[1]);
        break;
    case 'R':
        turnRight(str[1]);
        break;
    case 'I':
        brake();
        break;
    }
}
