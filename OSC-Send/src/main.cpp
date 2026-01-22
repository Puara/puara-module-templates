//****************************************************************************//
// OSC-Send template                                                          //
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

// Initialize Puara's module manager
Puara puara;

// UDP instances to let us send and receive packets
WiFiUDP Udp;

std::string oscIP_1{};
int oscPort_1{};

// Dummy sensor data used as example
float sensor;

// DEFINE FUNCTION SIMPLY FOR USERS HERE
void onSettingsChanged() {
    Udp.begin(puara.getVarNumber("localPORT"));
}

void setup() {
    #ifdef Arduino_h  // try serial.begin without this ifdef
        Serial.begin(115200);
    #endif

/* puara.start() initializes the filesystem with given configurations and settings.
 * The process then tries to connect to the WiFi Network (SSID) defined in config.json. 
 * User may change default SSID values. If process cannot find SSID, it will create it's
 * own WiFi Access Point (AP) to which user may connect and recieve it's OSC messages.
 * The process will also start a webserver where user may modify configurations and 
 * settings using any browser. Finally, the process instantiates serial listening, 
 * MDNS service, and scans for surrounding WiFi networks.
 */
    puara.start();
    Udp.begin(puara.getVarNumber("localPORT"));

    // This allows us to reconfigure the UDP reception port
    puara.set_settings_changed_handler(onSettingsChanged);

    // If needed, define your pins here. Refer to your board's documentation for appropriate 
    // pin numbers. These numbers are given as placeholder examples.

    // Setting a pin as input
    // pinMode(7, INPUT); // Sets pin 7 as an input pin

    // Setting a pin as an input with pull-up resistor
    // pinMode(2, INPUT_PULLUP); // Sets pin 2 as an input pin with an internal pull-up resistor enabled
}

void loop() {

    // Retrieve OSC settings from Puara. 
    // These can be changed in the web interface and will automatically update here.
    oscIP_1 = puara.getVarText("oscIP");
    oscPort_1 = puara.getVarNumber("oscPORT");

    // Update the dummy sensor variable with a random number
    sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));

    // If using actual sensors, read their values here instead of the dummy data.
    // Example for reading an analog sensor connected to pin A0
    // sensor = analogRead(A0); // Reads the value from the analog pin A0


    // print the dummy sensor data
    Serial.print("Dummy sensor value: ");
    Serial.println(sensor);

    /*
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the
     * network (WiFiUdp will print a warning message in those cases).
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
