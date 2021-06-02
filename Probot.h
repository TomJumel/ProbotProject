#ifndef CONFIG_H
#define CONFIG_H
//------------- H bridge motor
#define ROBOT_M1 //For one move motor
//#define ROBOT_M2 //For two motor move (Left and right)
#define ROBOT_H_PIN_A1 32
#define ROBOT_H_PIN_B1 33
//for ROBOT_M2
//#define ROBOT_H_PIN_A2 <pin>
//#define ROBOT_H_PIN_B2 <pin>


#define SPEEDPIN 25 //speed pin of the motor
#define SPEED_ZERO 120 // pwm value when the motor don't move
//#define SPEED_MAX 255 //bridle for the motors



#define USE_HBRIDGE_DIRECTION
/**/#define HBRDIGE_LEFT_DIRECTION 19 //turn left control pin
/**/#define HBRDIGE_RIGHT_DIRECTION 18 // turn right control pin
/**/#define HBRDIGE_SPEED_DIRECTION 5 // speed control
/**/#define HBRIDGE_MOTOR_MIN 150 // motor min for turning
#define ACTION_BUTTON_PIN 20
#endif