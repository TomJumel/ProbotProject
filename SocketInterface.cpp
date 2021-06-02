#include "SocketInterface.h"
#include "Arduino.h"
#include <AsyncUDP.h>

Communicator::Communicator(bool log)
{
  _log = log;
}

void Communicator::onDataNeedToBeSended(SendDataCallback sendCallback)
{
  _sendData = sendCallback;
}

void Communicator::init(Configurator *configurator, RobotMove *robotMove)
{
  _robot = robotMove;
  _configurator = configurator;
  if(true){
    initSta();
  }
}

void Communicator::initAp()
{
  print("Initialisation du AP ...");
  String macAddress = WiFi.macAddress();
  String ssid = "Probot" + macAddress;
  print(String("Generation du AP : ") + ssid);
  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP(ssid.c_str(), WiFi.macAddress().c_str());
  if (result)
  {
    print("AP genere");
  }
  else
  {
    print("Génération de l'AP à échoué, redémarrage demandé");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    while (1)
      yield();
  }
}

void Communicator::initSta()
{
  wifi_creds creds = _configurator->getCredentials();
  if (creds.isEmpty)
  {
    startConfigurationManager();
    return;
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    wl_status_t state = WiFi.begin(creds.ssid.c_str(), creds.pass.c_str());
    if(WL_CONNECT_FAILED == state){
      _configurator->factoryReset();
      ESP.restart();
    }
    int counter = 25;
    print("Connecting to WiFi : " + String(creds.ssid));
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print('.');
      delay(1000);
      if(0==--counter)
        break;
    }
    if(counter == 0)
      initAp();
  }
  return;
}
void Communicator::startConfigurationManager()
{
  AsyncUDP udpConfig;
  initAp();
  if (udpConfig.listen(1234))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.softAPIP());
    pinMode(1,OUTPUT);
    udpConfig.onPacket([&](AsyncUDPPacket packet) {
      String data = (const char *)packet.data();
      data = data.substring(0, packet.length());
      if (data[0] == (char)2)
      {
        data = data.substring(1);
        wifi_creds creds;
        creds.ssid = _slice(data, '!', 0);
        String pass = _slice(data, '!', 1);
        uint8_t idx = pass.indexOf((char)3);
        pass = pass.substring(0, idx);
        creds.pass = pass.c_str();
        _configurator->saveNetworkCredentials(creds);
        udpConfig.broadcastTo("\1", 1235);
        delay(50);
        udpConfig.broadcastTo("\1", 1235);
        delay(50);
        udpConfig.broadcastTo("\1", 1235);
        _configurator->save();
        ESP.restart();
      }
    });
    while (true)
    {
      yield();
    }
  }
}

void Communicator::commandReferral(String command)
{
  if (command[0] == 'I')
  {
    String inf = _configurator->getInfo();
    _sendData(inf.c_str(), inf.length());
    return;
  }
  switch (command[0])
  {
  case 'M':
    _robot->parseRequest(command.substring(1));
    break;
  case 'E': // Modif module
  case 'm': // application val module
    _configurator->moduleManager.parseRequest(command);
    if (command[0] == 'E')
      _configurator->save();
    break;
  case 'C':
      _sendData("\1",1);
    break;
  }
}

void Communicator::handler()
{
  if(needParsing){
    parseCommunication(rawPayload);
    needParsing = false;
  }
}

void Communicator::print(String info)
{
  if (!_log)
    return;
  Serial.printf("\n[Communicator] : %s" ,info.c_str());
}

void Communicator::parseCommunication(String payload)
{
  commandReferral(removeDelimiters(payload));
}

String Communicator::removeDelimiters(String rawData)
{
  if (rawData[0] == 2)
  {
    rawData = rawData.substring(1);
  }
  int last = 0;
  if ((last = rawData.lastIndexOf(3)) != -1)
  {
    return rawData.substring(0, last);
  }
  return rawData;
}

String Communicator::_slice(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
