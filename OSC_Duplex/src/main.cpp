//****************************************************************************//
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Puara Module : OSC-Receive template                                        //
//                                                                            //
// This template demonstrates how to set up a basic OSC receiver.             //
// Please refer to CNMAT's OSC repository on Github for more details on OSC.  //
//                                                                            //
// Puara Module Manager facilitates embedded sytem development by providing   //
// a set of pre-defined modules that manage filesystem, webserver and network //
// connections so the user can focus on easier prototyping of the system.     //
//                                                                            //
// This program will configure the filesystem with information found in       //
// config.json and settings.json files. The process will then try to connect  //
// to the WiFi Network (SSID) defined in config.json (STA mode).              //
// If the user wants the board to connect to a specific WiFi network, they    //
// may modify the "wifiSSID" and "wifiPSK" values in config.json file with    //
// the name of the desired WiFi network and it's password respectively.       //
//                                                                            //
// If the process cannot connect to a valid SSID, it will create it's own     //
// WiFi Access Point (AP mode) to which users may connect and send OSC        //
// messages to the board.                                                     //
//                                                                            //
// The process will then start a webserver that users may use to modify       //
// configurations and settings of their board/program without having to       //
// rebuild/reflash their program.                                             //
// In AP mode, access these webpages by connecting to the board's WiFi        //
// network, open a browser and type the network name followed by ".local" in  //
// the address bar. For example, if the board's network name is the default   //
// "Puara_001", type "puara_001.local" in the browser's address bar.          //
// If in STA mode, type the board's IP address in the browser's address bar.  //
//****************************************************************************//

#include "Arduino.h"
#include "puara.h"
#include <OSCMessage.h>
#include <WiFiUdp.h>

#include <iostream>

Puara puara;
WiFiUDP Udp;

std::string oscIP{};
int oscPort{};

// Dummy sensor data
float sensor;

void onSettingsChanged() {
    Udp.begin(puara.getVarNumber("localPORT"));
    oscIP = puara.getVarText("oscIP");
    oscPort = puara.getVarNumber("oscPORT");
}

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    /*
     * The Puara start function initializes the spiffs, reads the config and custom JSON
     * settings, start the wi-fi AP, connects to SSID, starts the webserver, serial
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();
    Udp.begin(puara.getVarNumber("localPORT"));

    // This allows us to reconfigure the UDP reception port
    puara.set_settings_changed_handler(onSettingsChanged);
    oscIP = puara.getVarText("oscIP");
    oscPort = puara.getVarNumber("oscPORT");
}

void loop() {


    // Update the dummy sensor variable with a random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

    // print the dummy sensor data
    Serial.print("Dummy sensor value: ");
    Serial.println(sensor);

    /*
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the
     * network (WiFiUdp will print a warning message in those cases).
     */
    if (!oscIP.empty() && oscIP != "0.0.0.0") { // set namespace and send OSC message for address 1
        OSCMessage msg1(("/" + puara.dmi_name()).c_str());
        msg1.add(sensor);
        Udp.beginPacket(oscIP.c_str(), oscPort);
        msg1.send(Udp);
        Udp.endPacket();
        msg1.empty();
        std::cout << "Message send to " << oscIP << ":" << oscPort << std::endl;
    }

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
