//****************************************************************************//
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Puara Module : OSC-Send template                                           //
//                                                                            //
// This template demonstrates how to set up a basic OSC transmitter.          //
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

// Dummy sensor data used as example
float sensor;

/*
 * The onSettingsChanged() function is called when settings are saved in the web
 * interface (click on "Save" button). This allows user to change variables on
 * their board without needing to go through the code build/flash process again.
 * Here we use it to change the UDP port for OSC reception and transmission.
 */
void onSettingsChanged() {
  Udp.begin(puara.getVarNumber("localPORT"));
  oscIP = puara.getVarText("oscIP");
  oscPort = puara.getVarNumber("oscPORT");
}

void setup() {
#ifdef Arduino_h
  Serial.begin(115200);
#endif
  puara.start();
  Udp.begin(puara.getVarNumber("localPORT"));
  puara.set_settings_changed_handler(onSettingsChanged);
  //  Retrieve OSC IP/PORT from user defined data. User may modify these in
  //  webpage
  oscIP = puara.getVarText("oscIP");
  oscPort = puara.getVarNumber("oscPORT");

  /*
   If needed, define your pins here. Refer to your board's documentation for
   appropriate pin numbers. The numbers given here are only placeholders.
  */

  /*  Example of setting pin 7 as an input. */
  // pinMode(7, INPUT);

  /*  Example of setting pin 2 as an input with internal pull-up resistor. */
  /*  Default value of pin 2 is now HIGH (True) when not connected to ground. */
  // pinMode(2, INPUT_PULLUP);
}

void loop() {

  /*
   If using actual sensors, read their values here instead of the dummy data.
  */

  /* Example for reading an analog sensor connected to pin 7 */
  // int sensor_analog = analogRead(7);

  /* Example for reading a digital signal (LOW/HIGH) connected to pin 2 */
  // int button = digitalRead(2);

  // Update and print the dummy sensor variable with a random number
  sensor = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 10));
  Serial.print("Dummy sensor value: ");
  Serial.println(sensor);

  /*
   * Sending OSC messages.
   * This sends the sensor value to the defined OSC IP : port.
   */
  if (!oscIP.empty() && oscIP != "0.0.0.0") {

    /* OSCmessage uses "device" and "id" fields from config.json file. */
    /* User may define these fields and must rebuild filesystem to change the */
    /* OSC address name. Default OSC address name is "Puara_001". */

    OSCMessage msg1(("/" + puara.dmi_name()).c_str());

    /* Add messages by appending to msg1 as shown below using msg1.add(). All */
    /* messages will be sent simultaneously in the same packet. */

    msg1.add(sensor);
    //  msg1.add(sensor_analog);
    //  msg1.add(button);

    /* To send a group of OSCMessage together, see OSCBundle in CNMAT's OSC
     * repo. */

    Udp.beginPacket(oscIP.c_str(), oscPort);
    msg1.send(Udp);
    Udp.endPacket();
    msg1.empty();
    std::cout << "Message send to " << oscIP << ":" << oscPort << std::endl;
  }

  /* For faster/slower transmission, manage speed of process here.            */
  /* This following tasks currently runs at 1 Hz (1 message per second).      */
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

/*
 * The Arduino header defines app_main and conflicts with having an app_main
 * function in code. This ifndef makes the code valid in case we remove the
 * Arduino header in the future.
 */
#ifndef Arduino_h
extern "C" {
void app_main(void);
}

void app_main() {
  setup();
  while (1) {
    loop();
  }
}
#endif
