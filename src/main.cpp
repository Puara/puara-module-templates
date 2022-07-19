//****************************************************************************//
// Puara Module Manager                                                       //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

// Include Puara's module manager
#include "puara.h"

// Include Puara's high-level descriptor library if planning to use the high-level
// descriptors' functions
#include "puara_gestures.h"

// Include Puara OSC functions
#include "puara_osc.h"

// declaring the libmapper device
mapper::Device* lm_dev;

// Initialize Puara's module manager
Puara puara;

// dummy sensor data
float sensor;
std::string sigName = "dummy_sensor";

// creating/declaring libmapper information on dummy sensor data
float lm_min = 0.0;
float lm_max = 10.0;
mapper::Signal dummy_signal;

void setup() {

    /**
     * the Puara start function initializes the spiffs, reads config and custom json
     * settings, start the wi-fi AP/connects to SSID, starts the webserver, serial 
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();

    // creating socket to send OSC messages
    startOSCArduino(puara.getLocalPORT());

    // creating the libmapper device
    lm_dev = new mapper::Device(puara.get_dmi_name());

    // Creating dummy signal
    dummy_signal = lm_dev->add_signal(mapper::Direction::OUTGOING, sigName, 1,
                                    mapper::Type::FLOAT, "un", &lm_min, &lm_max);
}

void loop() {
    // Poll libmapper
    lm_dev->poll();

    // Update dummy sensor with random number and send (OSC and libmapper)
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

    // updating libmapper dummy_signal
    dummy_signal.set_value(sensor);

    /* Sending OSC messages.
    *  If you're not planning to send messages to both addresses (OSC1 and OSC2),
    *  it is recommended to set the address to 0.0.0.0 to avoid cluttering the 
    *  network (WiFiUdp will print an warning message in those cases).
    */
    if (puara.IP1_ready()) {
        sendOSC(puara.getIP1(),puara.getPORT1(), puara.get_dmi_name(), sigName, sensor);
    }
    if (puara.IP2_ready()) {
        sendOSC(puara.getIP2(),puara.getPORT2(), puara.get_dmi_name(), sigName, sensor);
    }

    receiveOSC();
    // run at 100 Hz
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

/* The Arduino header defines app_main and conflicts with having an app_main function
*  in code. This ifndef makes the code valid in case we remove the Arduino header in
*  the future.
*/
#ifndef Arduino_h
    extern "C" {
        void app_main(void);
    }

    void app_main() {
        setup();
        loop();
    }
#endif
