//
// Created by Tom on 27/02/2021.
//

#ifndef PROGRAMM_MODULEMANAGER_H
#define PROGRAMM_MODULEMANAGER_H
#include <Arduino.h>
#include <ArduinoJson.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32PWM.h>
#else
#include <Servo.h>
#endif
#define MODULE_POOL_LENGTH 4
#define MODULE_PIN_LENGTH 6
typedef enum {
    ANALOG_PINTYPE, DIGITAL_PINTYPE, SERVO_PINTYPE
} PinType;
typedef struct {
    uint8_t pin;
    PinType type;
    Servo servo;
} Pin;
typedef struct {
    String name;
    uint8_t type;
    Pin pins[MODULE_PIN_LENGTH];
} Module;

class ModuleManager {
public:
    ModuleManager();
    void init();

    bool registerModule(Module module);
    bool unRegisterModule(Module module);
    bool moduleChange(String moduleName,Module newModule);

    bool moduleExistInPool(Module module);
    int8_t parseRequest(String request);
    Module decodeJson(String jsonString);
    String encodeJson(Module *module);
    void executeCommand(String json);
    void writeJsonPool(Stream *stream);
    Module* findModule(String name);
    uint8_t getNumberOfModules();
    void printInfoModule(Module module);
    void writeJsonPool(String *input);
private:
    Module modulesPool[MODULE_POOL_LENGTH];
    void loadModules();
    bool executeRequest(Pin* pin, uint8_t state);
    uint8_t _findNextModuleInPool(uint8_t start);
    String _slice(String data, char separator, int index);
};
/*Json structure module
 * { "name": "Mymodule", "pin": [ { "pin": 1, "pinType": 2 }, { "pin": 1, "pinType": 2 }, { "pin": 1, "pinType": 2 }, { "pin": 1, "pinType": 2 } ] }*/

#endif //PROGRAMM_MODULEMANAGER_H
