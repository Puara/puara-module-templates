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

/*
 The onSettingsChanged() function is called when settings are saved in the web
 interface (click on "Save" button). This allows user to execute changes in
 their firmware without needing to go through the build/flash process again.
 Here we use it to change the UDP port for OSC reception and transmission.
*/
void onSettingsChanged() { Udp.begin(puara.getVarNumber("localPORT")); }

void setup() {
#ifdef Arduino_h
  Serial.begin(115200);
#endif
  puara.start();
  Udp.begin(puara.getVarNumber("localPORT"));
  puara.set_settings_changed_handler(onSettingsChanged);
  /*
   If needed, define your pins here. Refer to your board's documentation for
   appropriate pin numbers. The numbers given here are only placeholders.
  */
  /*  Example of setting pin 7 as output.                                */
  // pinMode(7, OUTPUT);
}

void loop() {

  /*
   * Receiving OSC messages.
   * Please refer to CNMAT's OSC library documentation on Github for more
   * details.
   *
   * This template's example expects a float between [0,1] on the OSC address
   * "/led/brightness". Received message would have the following format :
   * /led/brightness f 0.34
   *
   * "led" and "brightness" are placeholders and can be changed for what user
   * prefers. At the time of this writing, these we're the supported data types
   * : int, int64, float, double, char, string, blob, boolean, time, rgba, midi,
   * event.
   *
   * Received OSC message arguments are accessed by their position. The first
   * argument is at position 0, the second at position 1, etc.
   * User may use functions such as getInt(0), getFloat(1), getString(2),
   * getRgba(3), getMidi(4) etc. to access the data in the message.
   *
   * UDP detail : as this template parses UDP message, try to contain your
   * exchanged messages between 500 and 1400 bytes to avoid fragmentation. If
   * your message is too big, it might be dropped (lost). For example, if
   * sending strings, send sentences rather than paragraphs.
   */
  OSCMessage inmsg;
  int size = Udp.parsePacket();
  while (size--) {
    inmsg.fill(Udp.read());
  }

  /* Process your received OSC message in here. */
  if (!inmsg.hasError()) {
    if (inmsg.fullMatch("/led/brightness") && inmsg.isFloat(0)) {

      // Example of using the received float at position 0 to set the brightness
      // of an LED on pin 7
      float value = inmsg.getFloat(0);
      int brightness =
          (int)(value * 255.0); // Assuming value is between 0.0 and 1.0
      // analogWrite(7, brightness);
      Serial.print("Writing brightness value to pin 7 : ");
      Serial.println(brightness);
    }
  }
  // For faster/slower transmission, manage speed of process here.
  // This following tasks currently runs at 1 Hz (1 message per second).
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
