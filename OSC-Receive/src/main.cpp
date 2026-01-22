//****************************************************************************//
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Puara Module : OSC-Receive template                                        //
//                                                                            //  
// This template demonstrates how to set up a basic OSC receiver.             //
//                                                                            //
// Puara Module Manager facilitates embedded sytem development by providing   //
// a set of pre-defined modules that manage filesystem, webserver and network //
// connections so the user can focus on easier prototyping of the system.     //
//                                                                            //
// This program will configure the filesystem with information found in       //
// config.json and settings.json files. The process will then try to connect  //
// to the WiFi Network (SSID) defined in config.json.                         //
// If the user wants the board to connect to a specific WiFi network, they may//
// modify the "wifiSSID" and "wifiPSK" values in config.json file with the    //
// name of the desired WiFi network and it's password respectively.           //
//                                                                            //  
// If the process cannot connect to a valid SSID, it will create it's own     //
// WiFi Access Point (AP) to which users may connect and send OSC messages    //
// to the board.                                                              // 
//                                                                            //  
// The process will also start a webserver where users may modify             //
// configurations and settings using any browser while being connected to AP. //
// To access the webpage, connect to the board's WiFi network, open a browser,//
// and type the network name followed by .local in the address bar.           //
// For example, if the board's network name is the default "Puara_001", type  //
// "puara_001.local" in the browser's address bar.                            //
//                                                                            //  
//****************************************************************************//

#include "Arduino.h"
#include "puara.h"
#include <WiFiUdp.h>
#include <OSCMessage.h>

#include <iostream>

Puara puara;
WiFiUDP Udp;

/*
 The onSettingsChanged() function is called when settings are saved in the web 
 interface (click on "Save" button). This allows user to execute changes in their
 firmware without needing to go through the build/flash process again. Here we 
 use it to change the UDP port for OSC reception and transmission.
*/
void onSettingsChanged() {
    Udp.begin(puara.getVarNumber("localPORT"));
}

void setup() {
    #ifdef Arduino_h  // try serial.begin without this ifdef
        Serial.begin(115200);
    #endif

    puara.start();
    Udp.begin(puara.getVarNumber("localPORT"));

    // This allows us to reconfigure the UDP reception port
    puara.set_settings_changed_handler(onSettingsChanged);

/*
 If needed, define your pins here. Refer to your board's documentation for 
 appropriate pin numbers. The numbers given here are only placeholders.
*/
    /* Example of setting pin 7 as an input. */
    // pinMode(7, INPUT);

    /* Example os setting pin 2 as an input with internal pull-up resistor. */
    /* Default value of pin is HIGH when not connected to ground.           */
    // pinMode(2, INPUT_PULLUP);
}

void loop() {


    // Update and print the dummy sensor variable with a random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));
    // Print the dummy sensor data on serial monitor
    Serial.print("Dummy sensor value: ");
    Serial.println(sensor);

/*
 If using actual sensors, read their values here instead of the dummy data.
*/
    // Example for reading an analog sensor connected to pin 7
    // int sensor_analog = analogRead(7);

    // Example for reading a digital signal (LOW/HIGH) connected to pin 2
    // int button = digitalRead(2);



    /*
     * Receiving OSC messages. This expects a float value on the OSC address /hi/there.
     */
    OSCMessage inmsg;
    int size = Udp.parsePacket();
    while (size--) {
        inmsg.fill(Udp.read());
    }
    if (!inmsg.hasError()) {
        if (inmsg.fullMatch("/hi/there") && inmsg.isFloat(0)) {
            Serial.print("got a float on address /hi/there : ");
            // you could receive more than one value in a message by checking inmsg.isFloat(1) or more.
            Serial.println(inmsg.getFloat(0));
        }
    }




    // For faster/slower transmission, manage speed of process here. 
    // This following tasks currently runs at 1 Hz (1 message per second).
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
