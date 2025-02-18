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

void loop() {

    // Update the dummy sensor variable with random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

    // print the dummy sensor data
    std::cout << "\n" 
    << "Settings stored in data/settings.json:\n" 
    << "Dummy sensor value: " << sensor << "\n"
    << std::endl;

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
