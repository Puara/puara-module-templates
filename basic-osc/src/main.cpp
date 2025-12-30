//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

#include "Arduino.h"

// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"

#include <iostream>

// Initialize Puara's module manager
Puara puara;

/*
 * Include CNMAT's OSC library (by Adrian Freed)
 * This library was chosen as it is widely used, but it can be replaced by any
 * other OSC library of choice
 */
#include <WiFiUdp.h>
#include <OSCMessage.h>

// UDP instances to let us send and receive packets
WiFiUDP Udp;

// Dummy sensor data
float sensor;
std::string oscIP_1;
int oscPort_1;

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
    Udp.begin(puara.getVarNumber("localPort"));
    oscIP_1 = puara.getVarText("oscIP");
    oscPort_1 = puara.getVarNumber("oscPort");
}

void loop() {

    // Update the dummy sensor variable with random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

    // print the dummy sensor data
    std::cout << "Dummy sensor value: " << sensor << std::endl;

    /* 
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the 
     * network (WiFiUdp will print an warning message in those cases).
     */
    if (!oscIP_1.empty() && oscIP_1 != "0.0.0.0") { // set namespace and send OSC message for address 1
        OSCMessage msg1(("/" + puara.dmi_name()).c_str()); 
        msg1.add(sensor);
        Udp.beginPacket(oscIP_1.c_str(), oscPort_1);
        msg1.send(Udp);
        Udp.endPacket();
        msg1.empty();
        std::cout << "Message send to " << oscIP_1 << ":" << oscPort_1 << std::endl;
    }

    // run at 1 Hz (1 message per second)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
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
