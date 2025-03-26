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

// Include puara structures to facilitare extracting gestures
#include "puara/structs.h"

// Include an IMU simulator to generate some data
#include "imu_simulator.h"

// Instatiate Puara's module manager
Puara puara;

// Instantiate a data holder (struct) to calculate the gestures
puara_gestures::Imu9Axis puaraIMU;

// Instatiate full orientation
IMU_Orientation orientation;

// Instatiate IMU-related gestures: jab, shake
// In this example, we tied the data holder to facilitate using the library
puara_gestures::Jab3D jab(&puaraIMU.accl);
puara_gestures::Shake3D shake(&puaraIMU.accl);

// Instatiate IMU simulator
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

    // Update the IMU simulated data...
    imu.update(); 

    // ... and storing the data into our holder
    puaraIMU.accl.x = imu.getAccelX();
    puaraIMU.accl.y = imu.getAccelY();
    puaraIMU.accl.z = imu.getAccelZ();
    puaraIMU.gyro.x = imu.getGyroX();
    puaraIMU.accl.y = imu.getGyroY();
    puaraIMU.accl.z = imu.getGyroZ();
    puaraIMU.magn.x = imu.getMagX();
    puaraIMU.magn.y = imu.getMagY();
    puaraIMU.magn.z = imu.getMagZ();

    // Print the simulated IMU data
    std::cout << "Simulated IMU data: [" 
    << puaraIMU.accl.x << ","
    << puaraIMU.accl.y << ","
    << puaraIMU.accl.z << ","
    << puaraIMU.gyro.x << ","
    << puaraIMU.accl.y << ","
    << puaraIMU.accl.z << ","
    << puaraIMU.magn.x << ","
    << puaraIMU.magn.y << ","
    << puaraIMU.magn.z << "]"
    << std::endl;

    // Call update to calculate each gesture
    // Since we tied our holder, the update function can be called without any argument
    jab.update();

    // Now we can access the current jab value with
    std::cout << "Jab: ["
    << jab.x.current_value() << jab.y.current_value() << jab.z.current_value() << "]" 
    << std::endl;

    // Alternatively, one can use directly evaluate the return of the update function  
    if (shake.update()) {
        std::cout << "Shake: ["
        << shake.x.current_value() << shake.y.current_value() << shake.z.current_value() << "]" 
        << std::endl;
    };

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
