#ifndef SOCKET_INTERFACE
#define SOCKET_INTERFACE
#include "Configurator.h"
#include "RobotMove.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32PWM.h>
#include <WiFi.h>
#define LED_BUILTIN 12
#else
#include <ESP8266WiFi.h>
#include <Servo.h>
#endif
typedef std::function<void(const char *dataToSend, int length)> SendDataCallback;
class Communicator
{
public:
    Communicator(bool log);
    void init(Configurator *configurator, RobotMove *robotMove);
    void initAp();
    void initSta();
    void startConfigurationManager();
    void parseCommunication(String payload);
    void onDataNeedToBeSended(SendDataCallback sendCallback);
    void handler();
    String removeDelimiters(String rawData);
    void commandReferral(String command);
    void requestParsing(String data){
        rawPayload = data;
        needParsing = true;
    }

    // void checkConnection();

private:
    bool _log;
    SendDataCallback _sendData;
    long _lastCheckMillis;
    RobotMove *_robot;
    Configurator *_configurator;
    void print(String info);
    static String _slice(String data, char separator, int index);
    bool needParsing = false;
    String rawPayload = "";
};

#endif
