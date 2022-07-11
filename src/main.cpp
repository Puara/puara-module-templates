//****************************************************************************//
// Puara Module Manager                                                       //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//


#include "module.h"

Puara puara;

extern "C" {
    void app_main(void);
}

void app_main() {

    //vTaskDelay(10000 / portTICK_PERIOD_MS);
    
    printf("\n");
    printf("**********************************************************\n");
    printf("* Puara Module Manager                                   *\n");
    printf("* Metalab - Société des Arts Technologiques (SAT)        *\n");
    printf("* Input Devices and Music Interaction Laboratory (IDMIL) *\n");
    printf("* Edu Meneses (2022) - https://www.edumeneses.com        *\n");
    printf("* Firmware version %d                                *\n", puara.get_version());                 
    printf("**********************************************************\n\n");


    puara.config_spiffs();
    
    puara.read_config_json();

    puara.read_settings_json();

    puara.start_wifi();

    puara.start_webserver();

    puara.start_serial_listening();

    puara.start_mdns_service(puara.get_dmi_name(), puara.get_dmi_name());

    puara.start_communication();

    puara.wifi_scan();

    printf("Done!\n");

    // dummy sensor data
    float sensor;

    puara.createMessage("dummy_sensor", 1, "un", 0.f, 10.f, "continuous");

    
    while (1) {
        // Poll libmapper
        puara.lmDev->poll();

        // Update dummy sensor data and send (OSC and libmapper)
        sensor = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10));
        puara.setMessage("dummy_sensor", sensor);

        // run at 100 Hz
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

}