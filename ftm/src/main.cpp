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

Puara puara;

uint8_t frame_count = 16;  // [16 (default), 24, 32, 64]
uint16_t burst_period = 2; // [0(No pref) 2- 255] in 100's of ms

//unsigned long ftm_request_start_time = 0;

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

    int num_responder_aps = puara.get_num_responder_aps();
    Serial.printf("Number of FTM responder APs detected in scan: %d\n", num_responder_aps);

    auto responders = puara.get_scanned_responder_aps();

    for (const auto& ap : responders) {
        Serial.printf("SSID: %s, BSSID: %02x:%02x:%02x:%02x:%02x:%02x, Channel: %d\n",
            ap.ssid.c_str(),
            ap.bssid[0], ap.bssid[1], ap.bssid[2], ap.bssid[3], ap.bssid[4], ap.bssid[5],
            ap.primary_channel);
    }

//    puara.configureFTM(frame_count, burst_period, uint8_t* target_bssid, uint8_t target_channel){

/*    puara.configureFTM(frame_count, burst_period); 

    // Verify if external AP / connected SSID is an FTM responder 
    if(!puara.get_ftm_responder_state()){
        Serial.println("External Access Point is not an FTM responder. Please connect to an FTM responder to perform ranging.");
    }

    // Send initial FTM request to trigger the first measurement
    ftm_request_start_time = millis();
    puara.requestFTM();
    */
}

void loop() {
/*
    if(puara.is_ftm_report_available()){
        // Calculate elapsed time since FTM request
        elapsed_ms = millis() - ftm_request_start_time;
    
        uint32_t distance_cm = puara.get_last_distance_cm();
        uint32_t rtt_ns = puara.get_last_rtt_ns();
        int rssi = puara.get_rssi_of_ftm_frame();
        
        // Print data 
        Serial.printf("Frame Count : %u, Burst Period : %u, Distance : %lu cm, RTT : %lu ns, RSSI : %d dBm, Elapsed Time : %lu ms\n",
             frame_count, burst_period, distance_cm, rtt_ns, rssi, elapsed_ms);
        
        puara.set_ftm_report_as_consumed();
        
        // Request next FTM measurement
        ftm_request_start_time = millis();
        puara.requestFTM();
    }

    */
    // Small yield to prevent tight busy-wait (optional but recommended)
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