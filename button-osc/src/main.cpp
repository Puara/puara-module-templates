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

// Include Puara-gestures:
// https://github.com/Puara/puara-gestures
#include "puara/gestures.h"

// Dummy button data
int button = 0; // Button state (0 or 1)
unsigned long previousMillis = 0;
long randomHoldTime = 0;
std::string oscIP_1;
int oscPort_1;

// This function updates the dummy button state based on non-blocking timing.
void updateButtonState() {
  // Get the current time.
  unsigned long currentMillis = millis();

  // Check if the random hold time has passed.
  if (currentMillis - previousMillis >= randomHoldTime) {
    // --- Time's up! Generate new random values. ---

    // Save the current time as the last update time.
    previousMillis = currentMillis;

    // Generate a new random state for the button: 0 or 1.
    button = random(2);

    // Generate a new random hold time in milliseconds (200ms to 5000ms).
    randomHoldTime = random(200, 5001);

    // Print the new state to the Serial Monitor for verification.
    Serial.print("New State: ");
    Serial.print(button);
    Serial.print(" | Holding for: ");
    Serial.print(randomHoldTime);
    Serial.println(" ms");
  }
}

// Instantiate the button gesture class
// In this example, we tied the data holder to facilitate using the library
puara_gestures::Button puara_button(&button);

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    // Seed the random number generator. Using analogRead() on an unconnected
    // pin provides a good random seed.
    randomSeed(analogRead(0));

    // --- Initialize with a starting random state ---
    // This ensures the simulation starts immediately without waiting.
    previousMillis = millis();
    button = random(2);
    randomHoldTime = random(200, 5001);

    /*
     * The Puara start function initializes the spiffs, reads the config and custom JSON
     * settings, start the wi-fi AP, connects to SSID, starts the webserver, serial
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();

    // Start the UDP instances
    Udp.begin(puara.getVarNumber("localPORT"));
    oscIP_1 = puara.getVarText("oscIP");
    oscPort_1 = puara.getVarNumber("oscPORT");
}

void loop() {

    // Update the dummy sensor variable with a random number
    updateButtonState();
    
     // update the puara-gestures button class
    puara_button.update();

    // print the dummy button data and puara-gestures
    std::cout << "\n"
    << "Dummy button value: "   << button    << "\n"
    << "gestures - count: "     << puara_button.count     << "\n"
    << "gestures - press: "     << puara_button.press     << "\n"
    << "gestures - tap: "       << puara_button.tap       << "\n"
    << "gestures - doubleTap: " << puara_button.doubleTap << "\n"
    << "gestures - tripleTap: " << puara_button.tripleTap << "\n"
    << "gestures - hold: "      << puara_button.hold      << "\n"
    << "gestures - pressTime: " << puara_button.pressTime << "\n"
    << std::endl;

    /*
     * Sending OSC messages.
     * If you're not planning to send messages to both addresses (OSC1 and OSC2),
     * it is recommended to set the address to 0.0.0.0 to avoid cluttering the
     * network (WiFiUdp will print a warning message in those cases).
     */
    if (!oscIP_1.empty() && oscIP_1 != "0.0.0.0") { // set namespace and send OSC message for address 1
        OSCMessage msg1(("/" + puara.dmi_name() + "/button").c_str());
        msg1.add(puara_button.count)
        .add(puara_button.press)
        .add(puara_button.tap)
        .add(puara_button.doubleTap)
        .add(puara_button.tripleTap)
        .add(puara_button.hold)
        .add(puara_button.pressTime);
        Udp.beginPacket(oscIP_1.c_str(), oscPort_1);
        msg1.send(Udp);
        Udp.endPacket();
        msg1.empty();
        
        std::cout << "Message send to " << oscIP_1 << ":" << oscPort_1 << std::endl;
    }


    // run at 100 Hz (100 messages per second)
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
