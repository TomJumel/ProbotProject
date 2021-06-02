//
// Created by Tom on 28/02/2021.
//

#include "Configurator.h"

uint8_t Configurator::init()
{
    print("Starting File System module...");
    Preferences preferences;
    preferences.begin("params");
    byte counter = preferences.getUChar("errorCounter",0);
    if (!SPIFFS.begin(counter==2))
    {
        counter++;
        preferences.putUChar("errorCounter",counter);
        print("Eror, File System won't begin");
        return 0;
    }
    if(counter == 2)
    preferences.putUChar("errorCounter",counter);
    if (isFirstStart())
    {
        Serial.println("First connection");
        return 1;
    }
    print("Discovering data ...");
    _gradMainData();
    print("Starting module discovery ...");
    uint8_t moduleCount = 0;
    if ((moduleCount= _grabModules())>0)
        print(String(moduleCount)+" module loaded !");
    else
        print("No module loaded");
    return 1;
}
wifi_creds Configurator::getCredentials()
{
    Preferences preferences = Preferences();
    preferences.begin("credentials", false);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    wifi_creds creds;
    creds.isEmpty = ssid == "" || password == "";
    creds.ssid = ssid.c_str();
    creds.pass = password.c_str();
    return creds;
}
void Configurator::saveNetworkCredentials(wifi_creds creds) {
  Preferences preferences = Preferences();
  preferences.begin("credentials", false);
  preferences.putString("ssid", creds.ssid);
  preferences.putString("password", creds.pass);
  preferences.end();
}
void Configurator::cleanPreferences() {
  Preferences preferences = Preferences();
  preferences.begin("credentials", false);
  preferences.clear();
  preferences.end();
}
void Configurator::factoryReset(){
    cleanPreferences();
    cleanFiles();
}
void Configurator::cleanFiles(){
    SPIFFS.remove(moduleDataFile);
    SPIFFS.remove(robotMainDataFile);
}
void Configurator::print(const String data)
{
    Serial.print("[Configurator] : ");
    Serial.print(data);
    Serial.println();
}

bool Configurator::_gradMainData()
{
    StaticJsonDocument<96> doc;
    File f = SPIFFS.open(robotMainDataFile, "r+");
    DeserializationError error = deserializeJson(doc, f);
    f.close();
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        name = "Probot";
        return false;
    }
    if (doc["name"] == "")
    {
        name = "Probot";
    }
    else
        name = doc["name"].as<String>();

    if (!doc["socketPort"])
    {
        socketPort = 81;
    }
    else
        socketPort = doc["socketPort"].as<int>();
    return true;
}

uint8_t Configurator::_grabModules()
{
    DynamicJsonDocument doc(1536);
    File f = SPIFFS.open(moduleDataFile, "r+");
    if (!f)
        return 0;
    DeserializationError error = deserializeJson(doc, f);
    if (error)
    {
        print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    }
    uint8_t counter = 0;
    for (JsonObject elem : doc.as<JsonArray>())
    {
        String output;
        serializeJson(elem, output);
        Module module = moduleManager.decodeJson(output);
        moduleManager.registerModule(module);
        counter++;
    }
    return counter;
}

void Configurator::save()
{
    print("Saving robot...");
    //Save modules
    print("Saving[1/2]");
    File file = SPIFFS.open(moduleDataFile, FILE_WRITE);
    if (!file)
        print("Impossible to save modules");
    moduleManager.writeJsonPool(&file);
    file.close();

    //Save robotData

    print("Saving[2/2]");
    file = SPIFFS.open(robotMainDataFile, FILE_WRITE);
    if (!file)
        print("Impossible to save Robot Settings");
    StaticJsonDocument<48> doc;
    doc["name"] = name;
    doc["socketPort"] = socketPort;
    serializeJson(doc, file);
    file.close();
}

bool Configurator::isFirstStart()
{
    return !SPIFFS.exists(moduleDataFile) && !SPIFFS.exists(robotMainDataFile);
}

String Configurator::getInfo()
{
    DynamicJsonDocument doc(1536);
    doc["name"] = name;
    doc["socketPort"] = socketPort;
    doc["robotState"] = String(SPIFFS.usedBytes()) + "/" + String(SPIFFS.totalBytes());
    String outStream;
    StringStream stream((String &)outStream);
    moduleManager.writeJsonPool(&stream);
    doc["modules"] = outStream;
    outStream = "";
    serializeJson(doc, stream);
    return outStream;
}
