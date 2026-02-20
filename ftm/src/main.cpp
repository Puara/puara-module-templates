//****************************************************************************//
// Puara Module Manager                                                       //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL)                     //
//                                                                            //      
//   How to use this template :                                               //  
//                                                                            //
//   You need two devices for fine time measurement FTM purposes.             //
//                                                                            //
// 1. Make a simple FTM Responder by flashing Puara Basic template on a first //
//    device that supports FTM. Only ESP32-S2, S3, C2, C3, C5, C6 support FTM //
//    (not original ESP32). Some routers also support FTM. Check if your      //
//    router supports the IEEE 802.11mc standard to act as an FTM responder.  //
//                                                                            //
// 2. Flash this template on the 2nd device, which will act as FTM Initiator. //
//                                                                            //
// ┌─────────────────────┐                      ┌─────────────────────┐       //
// │   FTM INITIATOR     │                      │   FTM RESPONDER     │       //
// │  (this template)    │                      │  (Puara Basic or    │       //
// │  ESP32-S2/S3/C2/    │                      │   802.11mc Router)  │       //
// │        C3/C5/C6     │                      │  ESP32-S2/S3/C2/    │       //
// │                     │                      │  C3/C5/C6 or Router │       //
// └─────────┬───────────┘                      └──────────┬──────────┘       //
//           │  1. FTM Request (t1)                        │                  //
//           │ ─────────────────────────────────────────►  │                  //
//           │  2. ACK (t2, t3)                            │                  //
//           │  ◄───────────────────────────────────────── │                  //
//           │  3. FTM Frame (t4)                          │                  //
//           │  ◄───────────────────────────────────────── │                  //
//           ▼                                             ▼                  //
//     ┌─────────────────────────────────────────────────────┐                //
//     │  Distance = c × (RTT / 2)                           │                //
//     │  RTT = (t4 - t1) - (t3 - t2)                        │                //
//     │  c = speed of light ≈ 3 × 10⁸ m/s                   │                //
//     └─────────────────────────────────────────────────────┘                //
//                                                                            //
// Initiator starts the measurement and calculates the distance               //
// Responder just replies with precise timestamps                             //
// Multiple bursts can improve accuracy (averaging)                           //
// Typical accuracy: ~1-2 meters indoors                                      //
//****************************************************************************//

#include "Arduino.h"
#include "puara.h"

#include <iostream>


// MLAT modes for different numbers of responders
#define UNILATERATION 1
#define BILATERATION 2
#define TRILATERATION 3
#define QUADRILATERATION 4
#define FIVE_POINT_MLAT 5
#define SIX_POINT_MLAT 6

// User selects the mode here:
#define MLAT_MODE BILATERATION

Puara puara;

uint8_t frame_count = 16;  // [16 (default), 24, 32, 64]
uint16_t burst_period = 2; // [0(No pref) 2- 255] in 100's of ms
unsigned long ftm_request_start_time = 0;

std::map<std::string, wifi_ftm_initiator_cfg_t> responder_configs;

std::vector<std::string> return_list_of_SSID(int num_inputs);

/*
// Update FTM configuration and trigger new measurement when settings are changed/saved from the web interface
void onSettingsChanged() {

    // End ongoing FTM session to apply new settings immediately
    puara.end_ftm_request_session();
  
    // Discard any pending report from the old configuration
    if (puara.is_ftm_report_available()) {
        puara.set_ftm_report_as_consumed();
    }
    frame_count = puara.getVarNumber("frame count");
    burst_period = puara.getVarNumber("burst period");

    // Add a delay to help ensure the previous session is fully terminated before starting a new one
    vTaskDelay(100 / portTICK_PERIOD_MS);

    puara.configureFTM(frame_count, burst_period); 

    ftm_request_start_time = millis();
    puara.requestFTM(); // Trigger new FTM procedure with updated settings
}
*/
void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    puara.start(PuaraAPI::UART_MONITOR, ESP_LOG_VERBOSE);

    const auto returned_ssids = return_list_of_SSID(MLAT_MODE);

    // Return a map of responders defined by user and their wifi_ftm_cfg_t struct info (MAC, channel)
    responder_configs = puara.get_map_of_responder_configs(frame_count, burst_period, returned_ssids); 

}

void loop() {
    for(auto& [ssid, cfg] : responder_configs) {
        Serial.printf("Responder SSID: %s\n", ssid.c_str());
        esp_wifi_ftm_initiate_session(&cfg);
        
        while(!puara.ftm_report_available()) {
            // Wait for the FTM report to be available
            vTaskDelay(1 / portTICK_PERIOD_MS); // Check every 1 ms
        }
        Serial.printf("Received FTM report for SSID: %s\n", ssid.c_str());
        puara.set_ftm_report_as_consumed();
    }
}



std::vector<std::string> return_list_of_SSID(int num_inputs) {
    
  std::vector<std::string> ssid_list;

  for (int i = 1; i <= num_inputs; ++i) {
    std::string key = "Responder" + std::to_string(i) + "_SSID";
    std::string ssid = puara.getVarText(key);
    if (!ssid.empty()) {
        Serial.printf("Retrieved SSID for Responder %d: %s\n", i, ssid.c_str());
      ssid_list.push_back(ssid);
    }
  }
  return ssid_list;
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