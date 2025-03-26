//****************************************************************************//
// Puara Template - RoArm-M2                                                  //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

/*
 * Warning: This firmware is distributed without any legal waranties or support.
 * The firmware is provided as is and you are responsible for any damages or losses
 * that may result from its use. The firmware is intended for educational and
 * research purposes only. You should not use the firmware in any situation where
 * it may cause harm or danger to any person or animal. You should not use the
 * firmware in any situation where it may cause damage to any property or object.
 * 
 * Parts of this firmware are based on the RoArm M2 firmware from Waveshare.
 * The original Waveshare RoArm M2 firmware can be found at
 * https://github.com/waveshareteam/roarm_m2.
 * 
 * Be careful when using the RoArm-M2. The RoArm-M2 is a powerful robotic arm that
 * can cause harm or damage if not used properly. Always be aware of the arm's
 * position and movement. Always be ready to stop the arm's movement, off the arm's 
 * power, or unplug the arm if it is behaving unexpectedly.
*/
#include "Arduino.h"

// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"

#include <iostream>

// RoArm-M2 dependencies
#include <ArduinoJson.h>
#include <SCServo.h>
StaticJsonDocument<256> jsonInfoHttp;

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

// Store and prepare to deal with errors
OSCErrorCode error;

/*
 * Functions for oled, RoArm-M2, pneumatic modules and lights ctrl
 * From https://github.com/waveshareteam/roarm_m2
*/
#include "oled_ctrl.h"
#include "RoArm-M2_module.h"
#include "switch_module.h"

// Calback function to handle the incoming OSC message
void led(OSCMessage &msg) {
    int ledState = msg.getInt(0);
    Serial.print("/led: ");
    Serial.println(ledState);
  }

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    delay(1200);

    /*
     * This is the start of the init for RoArm-M2
     * From https://github.com/waveshareteam/roarm_m2
     * The init functions are in the files oled_ctrl.h, RoArm-M2_module.h, and switch_module.h
     */
    initOLED();
    screenLine_0 = "Puara-Module / RoArm-M2";
    screenLine_1 = "SAT";
    screenLine_2 = "starting...";
    screenLine_3 = "";
    oled_update();

    /*
     * the Puara start function initializes the spiffs, reads config and custom json
     * settings, start the wi-fi AP/connects to SSID, starts the webserver, serial 
     * listening, MDNS service, and scans for WiFi networks.
     */    
    screenLine_2 = screenLine_3;
    screenLine_3 = "Initialize Puara-module";
    oled_update();
    if(InfoPrint == 1){Serial.println("Initialize LittleFS for Flash files ctrl.");}
    puara.start();
    // Start the UDP instances 
    Udp.begin(puara.LocalPORT());

    // init the funcs in switch_module.h
    screenLine_2 = screenLine_3;
    screenLine_3 = "Initialize 12V-switch ctrl";
    oled_update();
    if(InfoPrint == 1){Serial.println("Initialize the pins used for 12V-switch ctrl.");}
    switchPinInit();

    // servos power up
    screenLine_2 = screenLine_3;
    screenLine_3 = "Power up the servos";
    oled_update();
    if(InfoPrint == 1){Serial.println("Power up the servos.");}
    delay(500);
    
    // init servo ctrl functions.
    screenLine_2 = screenLine_3;
    screenLine_3 = "ServoCtrl init UART2TTL...";
    oled_update();
    if(InfoPrint == 1){Serial.println("ServoCtrl init UART2TTL...");}
    RoArmM2_servoInit();

    // check the status of the servos.
    screenLine_2 = screenLine_3;
    screenLine_3 = "Bus servos status check...";
    oled_update();
    if(InfoPrint == 1){Serial.println("Bus servos status check...");}
    RoArmM2_initCheck(false);

    if(InfoPrint == 1 && RoArmM2_initCheckSucceed){
        Serial.println("All bus servos status checked.");
    }
    if(RoArmM2_initCheckSucceed) {
        screenLine_2 = "Bus servos: succeed";
    } else {
        screenLine_2 = "Bus servos: " + 
        servoFeedback[BASE_SERVO_ID - 11].status +
        servoFeedback[SHOULDER_DRIVING_SERVO_ID - 11].status +
        servoFeedback[SHOULDER_DRIVEN_SERVO_ID - 11].status +
        servoFeedback[ELBOW_SERVO_ID - 11].status +
        servoFeedback[GRIPPER_SERVO_ID - 11].status;
    }
    screenLine_3 = ">>> Moving to init pos...";
    oled_update();
    RoArmM2_resetPID();
    RoArmM2_moveInit();

    screenLine_3 = "Reset joint torque to ST_TORQUE_MAX";
    oled_update();
    if(InfoPrint == 1){Serial.println("Reset joint torque to ST_TORQUE_MAX.");}
    RoArmM2_dynamicAdaptation(0, ST_TORQUE_MAX, ST_TORQUE_MAX, ST_TORQUE_MAX, ST_TORQUE_MAX);

    screenLine_3 = "RoArm-M2 started";
    oled_update();
    if(InfoPrint == 1){Serial.println("RoArm-M2 started.");}

    RoArmM2_handTorqueCtrl(300);

    /*
     * This is the end of the init for RoArm-M2
     */

}

void loop() {

    OSCMessage msg;
    int size = Udp.parsePacket();

    if (size > 0) {
        while (size--) {
            msg.fill(Udp.read());
        }
        if (!msg.hasError()) {
            msg.dispatch("/led", led);
        } else {
            error = msg.getError();
            Serial.print("error: ");
            Serial.println(error);
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
