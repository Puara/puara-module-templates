//****************************************************************************//
// Puara Module Manager - Auxiliary OSC/libmapper functions                   //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

#ifndef PUARAOSC_H
#define PUARAOSC_H

// The following libraries requires Arduino framework to compile
#include <Arduino.h>     // required for ArduinoOSC (OSCMessage)
#include <WiFiUdp.h>     // required for ArduinoOSC (OSCMessage)
#include <OSCMessage.h>
#include <mapper_cpp.h>  // libmapper

void startOSCArduino(int localPORT);
void sendOSC(char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data);
void sendOSC(const char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data);
void sendOSC(std::string ip, unsigned int port, std::string deviceName, std::string messageNamespace, float data);
void sendTrioOSC(char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data1, float data2, float data3);
void sendTrioOSC(const char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data1, float data2, float data3);
void sendTrioOSC(std::string ip, unsigned int port, std::string deviceName, std::string messageNamespace, float data1, float data2, float data3);
void receiveOSC();

#endif