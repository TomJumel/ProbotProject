#include "Probot.h"
#include "RobotMove.h"
#include "SocketInterface.h"
#include "ModuleManager.h"
#include "Configurator.h"
#ifdef ROBOT_M1
RobotMove robotMove = RobotMove(ROBOT_H_PIN_A1, ROBOT_H_PIN_B1, SPEEDPIN);
#elif ROBOT_M2
RobotMove robotMove = RobotMove(ROBOT_H_PIN_A1, ROBOT_H_PIN_B1, ROBOT_H_PIN_A2, ROBOT_H_PIN_B2, SPEEDPIN);
#else
#error H bridge not defined how Probot can move ?
#endif
Communicator comunicator = Communicator(true);
Configurator configurator;
void udpHandler();
void udpSetup();
void RF24Handler();
void RF24Setup();
void setup()
{
  Serial.begin(9600); // for logs
  configurator.init();
  robotMove.init();
#if defined(SPEED_ZERO)
  robotMove.setMinPWM(SPEED_ZERO);
#endif
  Serial.print("Start communicator :");
  comunicator.init(&configurator, &robotMove);
  udpSetup();
  udpHandler();
  //RF24Setup(); // to enable NRF24
}

void loop()
{
  comunicator.handler();
  if(Serial.available()){
    char c = Serial.read();
    if(c == 'r')
      configurator.factoryReset();
  }
  //RF24Handler(); //to enable NRF24


  //input all needed connection algorithme and call comunicator.parseCommunication([your data (String)]) to start parsing data;
}
// for a udp communication for Probot App :
#include <AsyncUDP.h>
AsyncUDP udp;
void udpHandler()
{
  if (udp.listen(configurator.socketPort))
  {
    Serial.printf("Starting listening on %s:%d", WiFi.localIP().toString().c_str(), configurator.socketPort);
    udp.onPacket([&](AsyncUDPPacket packet) {
      String data = (const char *)packet.data();
      data = data.substring(0, packet.length());
      comunicator.requestParsing(data); // request parsing for loop, because parsing and executing command here can create stack overflow on upd_async task
    });
  }
}
void udpSetup()
{
  comunicator.onDataNeedToBeSended([&](const char *payload, int length) {
    udp.broadcastTo(payload, configurator.socketPort + 1);
  });
}

//nrf24 communication
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(27, 26); // CE, CSN
const byte address[6] = "00001";
void RF24Setup()
{
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.setPayloadSize(128);
}
void RF24Handler()
{
  if (radio.available())
  {
    char buffer[128] = "";
    radio.read(&buffer, sizeof(buffer));
    if (strcmp(buffer,"")!=0) //filter reading error
      comunicator.requestParsing(String(buffer));
  }
}