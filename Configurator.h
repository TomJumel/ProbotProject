//
// Created by Tom on 28/02/2021.
//

#ifndef PROGRAMM_CONFIGURATOR_H
#define PROGRAMM_CONFIGURATOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ModuleManager.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Preferences.h>
typedef struct
{
    String ssid;
    String pass;
    bool isEmpty = false;
} wifi_creds;
class Configurator
{
public:
    ModuleManager moduleManager = ModuleManager();

    uint8_t init();
    void save();
    wifi_creds getCredentials();
    void saveNetworkCredentials(wifi_creds creds);
    bool isFirstStart();
    void cleanPreferences();
    void factoryReset();
    String getInfo();
    void cleanFiles();

    String name = "Probot";
    int socketPort = 1234;

private:
    void print(const String data);

    bool _gradMainData();

    uint8_t _grabModules();

    uint8_t moduleNumbers;

protected:
    const char *robotMainDataFile = "/robotData.txt";
    const char *moduleDataFile = "/modules.txt";
};

#endif //PROGRAMM_CONFIGURATOR_H


#ifndef _STRING_STREAM_H_
#define _STRING_STREAM_H_

#include <Stream.h>

class StringStream : public Stream
{
public:
    StringStream(String &s) : string(s), position(0) {}

    // Stream methods
    virtual int available() { return string.length() - position; }
    virtual int read() { return position < string.length() ? string[position++] : -1; }
    virtual int peek() { return position < string.length() ? string[position] : -1; }
    virtual void flush(){};
    // Print methods
    virtual size_t write(uint8_t c)
    {
        string += (char)c;
        return 1;
    };

private:
    String &string;
    unsigned int length;
    unsigned int position;
};

#endif // _STRING_STREAM_H_
