//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL)                     //
//****************************************************************************//

#include "Arduino.h"
#include "puara.h"

#include <iostream>

Puara puara;

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    puara.start(PuaraAPI::UART_MONITOR, ESP_LOG_VERBOSE);
    //puara.start();
    puara.configureFTM(); 

}

void loop() {

    puara.requestFTM();
    // run at 1 Hz (1 message per second)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

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
