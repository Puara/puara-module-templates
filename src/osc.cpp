//****************************************************************************//
// Puara Module Manager - Auxiliary OSC/libmapper functions (.c)              //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

#include "osc.h"

// initializing UDP class
WiFiUDP oscEndpoint; 

void startOSCArduino(int localPORT) {
  // creating socket to send OSC messages
  std::cout << "Starting ArduinoOSC... ";
  oscEndpoint.begin(localPORT);
  std::cout << "done!" << std::endl;
}

void sendOSC(char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void sendOSC(const char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendOSC(std::string ip, unsigned int port, std::string deviceName, std::string messageNamespace, float data) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip.c_str()) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName.c_str(), messageNamespace.c_str());
      OSCMessage msg(namespaceBuffer);
      msg.add(data);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendTrioOSC(char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data1, float data2, float data3) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data1);
      msg.add(data2);
      msg.add(data3);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendTrioOSC(const char* ip, unsigned int port, const char* deviceName, const char* messageNamespace, float data1, float data2, float data3) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName, messageNamespace);
      OSCMessage msg(namespaceBuffer);
      msg.add(data1);
      msg.add(data2);
      msg.add(data3);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void sendTrioOSC(std::string ip, unsigned int port, std::string deviceName, std::string messageNamespace, float data1, float data2, float data3) {
    IPAddress oscIP;
    IPAddress emptyIP(0,0,0,0);
    if (oscIP.fromString(ip.c_str()) && oscIP != emptyIP) {
      char namespaceBuffer[40];
      snprintf(namespaceBuffer,(sizeof(namespaceBuffer)-1),"/%s/%s", deviceName.c_str(), messageNamespace.c_str());
      OSCMessage msg(namespaceBuffer);
      msg.add(data1);
      msg.add(data2);
      msg.add(data3);
      oscEndpoint.beginPacket(oscIP, port);
      msg.send(oscEndpoint);
      oscEndpoint.endPacket();
      msg.empty();
  }
}

void receiveOSC() {

  char buffer[64];
  int messageSize;
  char messageType;

  OSCErrorCode error;
  OSCMessage msgReceive;
  int size = oscEndpoint.parsePacket();

  if (size > 0) {
    std::cout << "\nOSC message received" << std::endl;
    while (size--) {
      msgReceive.fill(oscEndpoint.read());
    }
    if (!msgReceive.hasError()) {
      messageSize = msgReceive.size();
      std::cout << "OSC message received: " << std::endl;
      msgReceive.getAddress(buffer);
      std::cout << buffer << " ";
      for (int i=0; i < messageSize; i++) {
        messageType = msgReceive.getType(i); 
        std::cout << messageType << " ";
        switch(messageType) {
          case 'i':
            std::cout << msgReceive.getInt(i) << " ";
            break;
          case 'f':
            std::cout << msgReceive.getFloat(i) << " ";
            break;
          case 's':
            msgReceive.getString(i, buffer, sizeof(buffer));
            std::cout << buffer << " ";
            break;
          default:
            std::cout << "? ";
        }
      }
      std::cout << std::endl;
    }
  } //else {
  //  error = msgReceive.getError();
  //  std::cout << "\nOSC receive error: " << error << std::endl;
  //}
}