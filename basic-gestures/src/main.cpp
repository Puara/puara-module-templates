//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

#include "Arduino.h"
#include <iostream>

// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"

// Include Puara-gestures:
// https://github.com/Puara/puara-gestures
#include "puara/gestures.h"

// Include an IMU simulator to generate some data
#include "imu_simulator.h"

// Instatiate Puara's module manager
Puara puara;

// Instatiate UMI simulator
IMUSimulator imu;

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    // Initialize the IMU simulator
    imu.begin(); 

    /*
     * the Puara start function initializes the spiffs, reads config and custom json
     * settings, start the wi-fi AP/connects to SSID, starts the webserver, serial 
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();

    /* 
     * Printing custom settings stored. The data/config.json values will print during 
     * Initialization (puara.start)
     * Comment this part if you want to run on Wokwi. Wokwi currently does not supprot SPIFFS
     */
    // std::cout << "\n" 
    // << "Settings stored in data/settings.json:\n" 
    // << "Hitchhiker: "           << puara.getVarText  ("Hitchhiker")           << "\n"
    // << "answer_to_everything: " << puara.getVarNumber("answer_to_everything") << "\n"
    // << "variable3: "            << puara.getVarNumber("variable3")            << "\n"
    // << std::endl;
}

void loop() {

    // Update the IMU simulated data
    imu.update(); 

    // print the simulated IMU data
    std::cout << "\n" 
    << "Smilated IMU data: [" 
    << imu.getAccelX() << ","
    << imu.getAccelY() << ","
    << imu.getAccelZ() << ","
    << imu.getGyroX()  << ","
    << imu.getGyroY()  << ","
    << imu.getGyroZ()  << ","
    << imu.getMagX()   << ","
    << imu.getMagY()   << ","
    << imu.getMagZ()   << "]"
    << std::endl;

    // run at ~100 Hz
    vTaskDelay(10 / portTICK_PERIOD_MS);
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
