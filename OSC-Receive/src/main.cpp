//****************************************************************************//
// OSC-Receive template                                                       //
// ADD SIMPLE DEFINITION HERE                                                 //
//                                                                            //
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
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
 interface (click on "Save" button). This allows user to change variables on their
 board without needing to go through the code build/flash process again. Here we 
 use it to change the UDP port for OSC reception and transmission.
*/
void onSettingsChanged() {
    Udp.begin(puara.getVarNumber("localPORT"));
}

void setup() {
    #ifdef Arduino_h  // try serial.begin without this ifdef
        Serial.begin(115200);
    #endif

/* 
 puara.start() initializes the filesystem with given configurations and settings.
 The process then tries to connect to the WiFi Network (SSID) defined in config.json. 
 User may change default SSID values. If process cannot find SSID, it will create 
 it's own WiFi Access Point (AP) to which user may connect and receive it's OSC 
 messages. 
 The process will also start a webserver where user may modify configurations and 
 settings using any browser while being connected to AP. 
 Finally, the process instantiates serial listening, MDNS service, and scans for 
 surrounding WiFi networks.
 */
    puara.start();
    Udp.begin(puara.getVarNumber("localPORT"));

    // This allows us to reconfigure the UDP reception port
    puara.set_settings_changed_handler(onSettingsChanged);

/*
 If needed, define your pins here. Refer to your board's documentation for 
 appropriate pin numbers. The numbers given here are only placeholders.
*/
    /* Setting pin 7 as an input. */
    // pinMode(7, INPUT);

    /* Setting pin 2 as an input with internal pull-up resistor.    */
    /* Default value of pin is HIGH when not connected to ground.   */
    // pinMode(2, INPUT_PULLUP);
}

void loop() {

/* 
 Retrieve OSC IP and PORT from stored data. User may modify IP/PORT 
 values in webpage and these changes will update automatically here.
*/    
    oscIP_1 = puara.getVarText("oscIP");
    oscPort_1 = puara.getVarNumber("oscPORT");

    // Update and print the dummy sensor variable with a random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

/*
 If using actual sensors, read their values here instead of the dummy data.
*/
    // Example for reading an analog sensor connected to pin 7
    // int sensor_analog = analogRead(7);

    // Example for reading a digital signal (LOW/HIGH) connected to pin 2
    // int button = digitalRead(2);

    // Print the dummy sensor data on serial monitor
    Serial.print("Dummy sensor value: ");
    Serial.println(sensor);


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
