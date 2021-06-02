//
// Created by Tom on 27/02/2021.
//

#include "ModuleManager.h"

ModuleManager::ModuleManager() {
   for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        modulesPool[i].name = "";
    }
}

bool ModuleManager::registerModule(Module module) {
    for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        if (modulesPool[i].name == "") {
            modulesPool[i] = module;
            for(Pin cur : module.pins){
                if(cur.type == DIGITAL_PINTYPE && cur.pin < 70){
                    pinMode(cur.pin,OUTPUT);
                }
            }
            Serial.printf("Register module :%s\n", module.name.c_str());
            return true;
        }
    }
    return false;
}

bool ModuleManager::unRegisterModule(Module module) {
    for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        if (modulesPool[i].name != "")
            if (modulesPool[i].name == module.name) {
                modulesPool[i] = Module();
                Serial.printf("UnRegister module :%s\n", module.name.c_str());
                return true;
            }
    }
    return false;
}
bool ModuleManager::moduleChange(String moduleName, Module newModule){
    for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        if (modulesPool[i].name != "")
            if (modulesPool[i].name == moduleName) {
                modulesPool[i] = newModule;
                Serial.printf("Changing module :%s\n", modulesPool[i].name.c_str());
                return true;
            }
    }
    return false;
}
bool ModuleManager::moduleExistInPool(Module module) {
    for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        if (modulesPool[i].name != "")
            if (modulesPool[i].name == module.name) {
                return true;
            }
    }
    return false;
}
bool ModuleManager::executeRequest(Pin *pin, uint8_t state) {
    switch (pin->type) {
        case ANALOG_PINTYPE:
            analogWrite(pin->pin, state);
            return true;
        case DIGITAL_PINTYPE:
            digitalWrite(pin->pin, state);
            return true;
        case SERVO_PINTYPE:
            pin->servo.attach(pin->pin);
            pin->servo.write(state);
            return true;
    }
    return false;
}

Module ModuleManager::decodeJson(String jsonString) {
    Module module;
    module.name = "";
    StaticJsonDocument<384> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return module;
    }

    module.name = doc["name"].as<String>();
    module.type = doc["type"].as<uint8_t>();
    uint8_t cpt = 0;
    for (JsonObject elem : doc["pin"].as<JsonArray>()) {
        module.pins[cpt].pin = elem["pin"].as<uint8_t>();
        module.pins[cpt].type = elem["pinType"].as<PinType>();
        cpt++;
    }
    for (cpt = cpt; cpt < MODULE_PIN_LENGTH; cpt++) {
        module.pins[cpt].pin = 200;
        module.pins[cpt].type = DIGITAL_PINTYPE;
    }
    return module;
}

String ModuleManager::encodeJson(Module *module) {
    StaticJsonDocument<384> doc;

    doc["name"] = String(module->name);
    doc["type"] = module->type;

    JsonArray pin = doc.createNestedArray("pin");
    for (Pin curr : module->pins) {
        if(curr.pin==200)
        break;
        JsonObject pinObj = pin.createNestedObject();
        pinObj["pin"] = curr.pin;
        pinObj["pinType"] = curr.type;
    }
    String output;
    serializeJson(doc, output);
    return output;
}

int8_t ModuleManager::parseRequest(String request) {
    if (request[0] == 'E') {
        Module module = decodeJson(request.substring(2));
        if (module.name == "") {
            return -1;
        }
        if (request[1] == 'R') {
            return unRegisterModule(module)? 0 : -1;
        }
        if (request[1] == 'E') {
            return moduleChange(module.name,module) ? 0 : -1;
        }
        if (request[1] == 'r') {
            if(moduleExistInPool(module))
                return moduleChange(module.name,module) ? 0 : -1;
            return registerModule(module) ? 0 : -1;

        }
    } else if (request[0] == 'm') {
        request = request.substring(0);
        Module *mod = findModule(_slice(request,'&',1));
        if(mod){
            int pin = _slice(request,'&',2).toInt();
            int value = _slice(request,'&',3).toInt();
            for(Pin curr : mod->pins){
                if(curr.pin == pin)
                    return executeRequest(&curr,value)? 0 : -1;
            }
            return 12;
        }
        return 11;
    }
    return 1;
}
void ModuleManager::writeJsonPool(Stream *stream) {
    stream->print("[");
    uint8_t point = 0;
    while (point != MODULE_POOL_LENGTH){
        if(modulesPool[point].name != ""){
            stream->print(encodeJson(&modulesPool[point]));
            point = _findNextModuleInPool(point+1);
            if(point < MODULE_POOL_LENGTH){
                stream->print(',');
            }
        }else point++;
    }
    stream->print(']');
}
void ModuleManager::writeJsonPool(String *input) {
    (*input) = ("[");
    uint8_t point = 0;
    while (point != MODULE_POOL_LENGTH){
        if(modulesPool[point].name != ""){
            (*input) +=(encodeJson(&modulesPool[point]));
            point = _findNextModuleInPool(point+1);
            if(point < MODULE_POOL_LENGTH){
                (*input) += (',');
            }
        }else point++;
    }
    (*input) += (']');
}
uint8_t ModuleManager::_findNextModuleInPool(uint8_t start){
    int i;
    for (i = start; i < MODULE_POOL_LENGTH; ++i) {
        if(modulesPool[i].name != ""){
            return i;
        }
    }
    return MODULE_POOL_LENGTH;
}
Module* ModuleManager::findModule(String name) {
    for (byte i = 0; i < MODULE_POOL_LENGTH; i++) {
        if (modulesPool[i].name != "")
            if (modulesPool[i].name == name) {
                return &modulesPool[i];
            }
    }
    return NULL;
}
void ModuleManager::printInfoModule(Module module) {
    Serial.printf("Module : (\n\tname:%s,type:%d,\n\tpin:[",module.name.c_str(),module.type);
    for(Pin pin : module.pins)
        Serial.printf("\n\t(type:%d,pin:%d)",pin.type,pin.pin);
    Serial.print("\n\t])\n");
}
String ModuleManager::_slice(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}