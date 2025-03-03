//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

#include "Arduino.h"

// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"
#include "oscqwebserver.hpp"
#include <iostream>

// Initialize Puara's module manager
Puara puara;

/*
 * Include CNMAT's OSC library (by Adrian Freed)
 * This library was chosen as it is widely used, but it can be replaced by any
 * other OSC library of choice
 */
#include <WiFiUdp.h>

// UDP instances to let us send and receive packets
WiFiUDP Udp;

// Dummy sensor data
float sensor;

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    /*
     * the Puara start function initializes the spiffs, reads config and custom json
     * settings, start the wi-fi AP/connects to SSID, starts the webserver, serial
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();
    vTaskDelay(500  / portTICK_RATE_MS);
    // setup the oscquery webserver, mdns service and UDP instance.
    setup_oscquery_server(puara.getLocalPORT());
    /*
     * Printing custom settings stored. The data/config.json values will print during
     * Initialization (puara.start)
     */
    std::cout << "\n"
    << "Settings stored in data/settings.json:\n"
    << "Hitchhiker: "           << puara.getVarText ("Hitchhiker")            << "\n"
    << "answer_to_everything: " << puara.getVarNumber("answer_to_everything") << "\n"
    << "variable3: "            << puara.getVarNumber("variable3")            << "\n"
    << std::endl;
}

void process_value() {
  digitalWrite(BUILTIN_LED, ledState);
  Serial.print("/some_value: ");
  Serial.println(msg.getInt(0));
}

void loop() {
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      // You can add more dispatches here to react to other messages
      msg.dispatch("/some_value", process_value);
    } else {
      error = msg.getError();
      Serial.println(error);
    }
  }

}

/*
 * The Arduino header defines app_main and conflicts with having an app_main function
 * in code. This ifndef makes the code valid in case we remove the Arduino header in
 * the future.
 */
#ifndef Arduino_h
    extern "C" {
        void app_main(void);
    }

    void app_main() {
        setup();
        while(1) {
            loop();
        }
    }
#endif
