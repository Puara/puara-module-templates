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

//for test purposes - logging to file after upload -- run: 
//$ pio run -t upload -t monitor | tee -a ftm_data_cm.csv

#include "Arduino.h"
#include "puara.h"
#include "ftm_utils.h"

#include <iostream>

Puara puara;

// Configuration pairs for automated sampling (frame_count, burst_period)
// Focused on configs that complete in under 2 seconds
struct FTMConfig {
    uint8_t frame_count;
    uint16_t burst_period;
};

const FTMConfig configs[] = {
    // frame_count=16: burst_periods 2,4,6,8,10,12,14,16
    {16, 2}, {16, 4}, {16, 6}, {16, 8}, {16, 10}, {16, 12}, {16, 14}, {16, 16},
    // frame_count=24: burst_periods 2,4,6
    {24, 2}, {24, 4}, {24, 5},
    // frame_count=32: burst_period 2
    {32, 2},
    // frame_count=64: burst_period 2
    {64, 2}
};
const size_t NUM_CONFIGS = sizeof(configs) / sizeof(configs[0]);  // 13
const int SAMPLES_PER_CONFIG = 1000;

// Current configuration state
size_t config_idx = 0;
int sample_count = 0;
bool sampling_complete = false;

// Current active configuration values
uint8_t frame_count;
uint16_t burst_period;

int physical_distance = 300; // cm 

// Timing measurement variables
unsigned long ftm_request_start_time = 0;
const unsigned long FTM_TIMEOUT_MS = 5000; // Timeout after 5 seconds

// Function to advance to next configuration
void advanceToNextConfig() {
    sample_count = 0;
    config_idx++;
    
    if (config_idx >= NUM_CONFIGS) {
        sampling_complete = true;
        Serial.println("# Sampling complete! All configurations tested.");
        return;
    }
    
    frame_count = configs[config_idx].frame_count;
    burst_period = configs[config_idx].burst_period;
    puara.configureFTM(frame_count, burst_period);
    
    Serial.printf("# Config %zu/%zu: frame_count=%u, burst_period=%u\n", 
                  config_idx + 1, NUM_CONFIGS, frame_count, burst_period);
}

/*
void onSettingsChanged() {

    // End ongoing FTM session to apply new settings immediately
    puara.end_ftm_request_session();
  
    // Discard any pending report from the old configuration
    if (puara.is_ftm_report_available()) {
        puara.set_ftm_report_as_consumed();
    }
    frame_count = puara.getVarNumber("frame count");
    burst_period = puara.getVarNumber("burst period");
    physical_distance = puara.getVarNumber("manual FTM offset");
    puara.configureFTM(frame_count, burst_period); 
    //Serial.printf("Settings updated - frames: %u, burst: %u\n", frame_count, burst_period);
    ftm_request_start_time = millis();
    puara.requestFTM(); // Trigger new FTM procedure with updated settings
}
*/
void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    //puara.start(PuaraAPI::UART_MONITOR, ESP_LOG_VERBOSE);
    puara.start();

    // Initialize first configuration
    frame_count = configs[0].frame_count;
    burst_period = configs[0].burst_period;

    /* Configure FTM : (number of frames sent [16 (default), 24, 32, 64], burst mode [])
    //    Arguments and default values : (frame count = 32, burst period = 4)
    //    Frame Count :   No. of FTM frames requested in terms of 4 or 8 bursts 
    //                    (allowed values - 0(No pref), 16, 24, 32, 64) 
    //    Burst Period : Requested period between FTM bursts in 100's of milliseconds 
    //                   (allowed values 0(No pref) 2- 255) 
    */
    puara.configureFTM(frame_count, burst_period); 

    // Verify if external AP / connected SSID is an FTM responder 
    if(!puara.get_ftm_responder_state()){
        Serial.println("# External Access Point is not an FTM responder. Please connect to an FTM responder to perform ranging.");
    }

    // Print CSV header for data logging
    Serial.println("physical_distance_cm,frame_count,burst_period,estimated_distance_cm,rtt_ns,elapsed_ms");
    Serial.printf("# Starting automated sampling: %zu configs x %d samples = %zu total\n", 
                  NUM_CONFIGS, SAMPLES_PER_CONFIG, NUM_CONFIGS * SAMPLES_PER_CONFIG);
    Serial.printf("# Config 1/%zu: frame_count=%u, burst_period=%u\n", 
                  NUM_CONFIGS, frame_count, burst_period);

    // Send initial FTM request to trigger the first measurement
    ftm_request_start_time = millis();
    puara.requestFTM();
}


void loop() {

    // Stop if all sampling is complete
    if (sampling_complete) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }

    // Check for timeout - if FTM request takes too long, retry
    unsigned long elapsed_ms = millis() - ftm_request_start_time;
    if (elapsed_ms > FTM_TIMEOUT_MS) {
        Serial.printf("# Timeout after %lu ms, retrying...\n", elapsed_ms);
        
        // Consume any pending (possibly error) report
        if (puara.is_ftm_report_available()) {
            puara.set_ftm_report_as_consumed();
        }
        
        // Retry the same configuration
        ftm_request_start_time = millis();
        puara.requestFTM();
        return;
    }

    if(puara.is_ftm_report_available()){
        // Calculate elapsed time since FTM request
        elapsed_ms = millis() - ftm_request_start_time;
    
        uint32_t distance_cm = puara.get_last_distance_cm();
        uint32_t rtt_ns = puara.get_last_rtt_ns();
        
        // Skip samples with zero/invalid readings (likely errors)
        if (distance_cm == 0 && rtt_ns == 0) {
            Serial.println("# Invalid reading (0,0), retrying...");
            puara.set_ftm_report_as_consumed();
            ftm_request_start_time = millis();
            puara.requestFTM();
            return;
        }

        // Print CSV data row
        Serial.printf("%d,%u,%u,%lu,%lu,%lu\n", physical_distance, frame_count, burst_period, distance_cm, rtt_ns, elapsed_ms);
        
        puara.set_ftm_report_as_consumed();
        
        // Increment sample count and check if we need to advance config
        sample_count++;
        if (sample_count >= SAMPLES_PER_CONFIG) {
            advanceToNextConfig();
            if (sampling_complete) {
                return;
            }
        }
        
        // Request next FTM measurement
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