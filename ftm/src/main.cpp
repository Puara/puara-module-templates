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

uint8_t frame_count = 64;
uint16_t burst_period = 10;

// Timing measurement variables
unsigned long ftm_request_start_time = 0;

// Helper function to format elapsed time as min:sec:ms
void formatElapsedTime(unsigned long elapsed_ms, char* buffer, size_t buffer_size) {
    unsigned long minutes = elapsed_ms / 60000;
    unsigned long seconds = (elapsed_ms % 60000) / 1000;
    unsigned long milliseconds = elapsed_ms % 1000;
    snprintf(buffer, buffer_size, "%02lu:%02lu:%03lu", minutes, seconds, milliseconds);
}

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    //puara.start(PuaraAPI::UART_MONITOR, ESP_LOG_VERBOSE);
    puara.start();

    /* Configure FTM : (number of frames sent [16 (default), 24, 32, 64], burst mode [])
    //    Arguments and default values : (frame count = 32, burst period = 4)
    //    Frame Count :   No. of FTM frames requested in terms of 4 or 8 bursts 
    //                    (allowed values - 0(No pref), 16, 24, 32, 64) 
    //    Burst Period : Requested period between FTM bursts in 100's of milliseconds 
    //                   (allowed values 0(No pref) - 100) 
    */
    puara.configureFTM(frame_count, burst_period); 


    // Verify if external AP / connected SSID is an FTM responder 
    if(!puara.get_ftm_responder_state()){
        Serial.println("External Access Point is not an FTM responder. Please connect to an FTM responder to perform ranging.");
    }

    // Send initial FTM request to trigger the first measurement and get the first report.
    ftm_request_start_time = millis();
    puara.requestFTM();
}


void loop() {


    if(puara.is_ftm_report_available()){
        // Calculate elapsed time since FTM request
        unsigned long elapsed_ms = millis() - ftm_request_start_time;
        char time_buffer[16];
        formatElapsedTime(elapsed_ms, time_buffer, sizeof(time_buffer));

        uint32_t distance_cm = puara.get_last_distance_cm();
        uint32_t rtt_ns = puara.get_last_rtt_ns();
        
        Serial.printf("FTM Report - Distance: %u cm, RTT: %u ns, Response Time: %s (frames: %u, burst: %u)\n", 
                      distance_cm, rtt_ns, time_buffer, frame_count, burst_period);
        
        puara.set_ftm_report_as_consumed();
        
        // Trigger next FTM procedure and record start time
        ftm_request_start_time = millis();
        puara.requestFTM();
    }

    // Small yield to prevent tight busy-wait (optional but recommended)
    vTaskDelay(1 / portTICK_PERIOD_MS);
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



/* POSSIBLE API FUNCTIONS TO EXPOSE:
Configure FTM (Fine Timing Measurement) session parameters (e.g., number of measurements, burst mode, etc.)

Check last FTM status from another function / expose status to web UI 
Use : 
    wifi_ftm_status_t wifi_ftm_status;
  /*typedef enum {                      confirm these values with esp-idf docs 5.5.2
    FTM_STATUS_SUCCESS = 0,     // FTM exchange is successful 
    FTM_STATUS_UNSUPPORTED,     // Peer does not support FTM 
    FTM_STATUS_CONF_REJECTED,   // Peer rejected FTM configuration in FTM Request 
    FTM_STATUS_NO_RESPONSE,     // Peer did not respond to FTM Requests 
    FTM_STATUS_FAIL,            // Unknown error during FTM exchange 
} wifi_ftm_status_t;

*/