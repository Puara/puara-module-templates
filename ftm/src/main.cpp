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

// Physical distances to sample (in cm)
const int distances[] = {150, 300, 450, 600, 750, 900, 1050, 1200, 1350, 1500, 1650, 1800, 1950, 2100, 2250, 2400, 2550};
const size_t NUM_DISTANCES = sizeof(distances) / sizeof(distances[0]);  // 17

// Configuration pairs for automated sampling (frame_count, burst_period)
// Focused on configs that complete in under 2 seconds
struct FTMConfig {
    uint8_t frame_count;
    uint16_t burst_period;
};

const FTMConfig configs[] = {
    // configurations that give an answer in less than 1 second based on prior testing
    // frame_count=16: burst_periods 2,4,6
    {16, 2}, {16, 4}, {16, 6},
    // frame_count=24: burst_periods 2
    {24, 2}
};
const size_t NUM_CONFIGS = sizeof(configs) / sizeof(configs[0]);  // 13
const int SAMPLES_PER_CONFIG = 1000;

// Current state
size_t distance_idx = 0;
size_t config_idx = 0;
int sample_count = 0;
bool sampling_complete = false;
bool waiting_for_user = false;  // Flag to wait for user confirmation between distances
bool all_distances_complete = false;

// Current active configuration values
uint8_t frame_count;
uint16_t burst_period;
int physical_distance; 

// Timing measurement variables
unsigned long ftm_request_start_time = 0;
const unsigned long FTM_TIMEOUT_MS = 5000; // Timeout after 5 seconds

// Function to advance to next configuration
void advanceToNextConfig() {
    sample_count = 0;
    config_idx++;
    
    if (config_idx >= NUM_CONFIGS) {
        // All configs done for this distance
        sampling_complete = true;
        
        // Check if there are more distances
        if (distance_idx + 1 < NUM_DISTANCES) {
            Serial.println("#");
            Serial.printf("# ========== DISTANCE %d cm COMPLETE ==========\n", physical_distance);
            Serial.printf("# Next distance: %d cm\n", distances[distance_idx + 1]);
            Serial.println("# Move device to next position, then press ENTER to continue...");
            Serial.println("#");
            waiting_for_user = true;
        } else {
            all_distances_complete = true;
            Serial.println("#");
            Serial.println("# ==========================================");
            Serial.println("# ALL DISTANCES AND CONFIGURATIONS COMPLETE!");
            Serial.println("# ==========================================");
        }
        return;
    }
    
    frame_count = configs[config_idx].frame_count;
    burst_period = configs[config_idx].burst_period;
    puara.configureFTM(frame_count, burst_period);
    
    Serial.printf("# Config %zu/%zu: frame_count=%u, burst_period=%u\n", 
                  config_idx + 1, NUM_CONFIGS, frame_count, burst_period);
}

// Function to advance to next distance
void advanceToNextDistance() {
    distance_idx++;
    physical_distance = distances[distance_idx];
    config_idx = 0;
    sample_count = 0;
    sampling_complete = false;
    waiting_for_user = false;
    
    // Reset to first config
    frame_count = configs[0].frame_count;
    burst_period = configs[0].burst_period;
    puara.configureFTM(frame_count, burst_period);
    
    Serial.println("#");
    Serial.printf("# ========== STARTING DISTANCE %zu/%zu: %d cm ==========\n", 
                  distance_idx + 1, NUM_DISTANCES, physical_distance);
    Serial.printf("# Config 1/%zu: frame_count=%u, burst_period=%u\n", 
                  NUM_CONFIGS, frame_count, burst_period);
    
    // Start sampling
    ftm_request_start_time = millis();
    puara.requestFTM();
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

    // Verify if external AP / connected SSID is an FTM responder 
    if(!puara.get_ftm_responder_state()){
        Serial.println("# External Access Point is not an FTM responder. Please connect to an FTM responder to perform ranging.");
    }

    // Display available distances and prompt user to select starting position
    Serial.println("#");
    Serial.println("# ==========================================");
    Serial.println("# FTM CALIBRATION - DISTANCE SELECTION");
    Serial.println("# ==========================================");
    Serial.println("# Available distances (cm):");
    for (size_t i = 0; i < NUM_DISTANCES; i++) {
        Serial.printf("#   %zu: %d cm\n", i, distances[i]);
    }
    Serial.println("#");
    Serial.println("# Enter the index (0-16) of starting distance, then press ENTER:");
    Serial.println("# (Default: 0 for 150cm if no input within 60 seconds)");
    
    // Wait for user input with timeout
    unsigned long start_wait = millis();
    String input = "";
    while (millis() - start_wait < 60000) {  // 60 second timeout
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                break;
            }
            if (c >= '0' && c <= '9') {
                input += c;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // Parse input or use default
    if (input.length() > 0) {
        int idx = input.toInt();
        if (idx >= 0 && idx < (int)NUM_DISTANCES) {
            distance_idx = idx;
        } else {
            Serial.printf("# Invalid index %d, using 0\n", idx);
            distance_idx = 0;
        }
    } else {
        Serial.println("# No input received, using default (index 0)");
        distance_idx = 0;
    }
    
    physical_distance = distances[distance_idx];
    Serial.printf("# Selected starting distance: %d cm (index %zu)\n", physical_distance, distance_idx);
    Serial.println("#");

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

    // Print CSV header for data logging
    Serial.println("physical_distance_cm,frame_count,burst_period,estimated_distance_cm,rtt_ns,elapsed_ms");
    Serial.printf("# ========== STARTING DISTANCE %zu/%zu: %d cm ==========\n", 
                  distance_idx + 1, NUM_DISTANCES, physical_distance);
    Serial.printf("# Sampling %zu configs x %d samples = %zu samples per distance\n", 
                  NUM_CONFIGS, SAMPLES_PER_CONFIG, NUM_CONFIGS * SAMPLES_PER_CONFIG);
    Serial.printf("# Config 1/%zu: frame_count=%u, burst_period=%u\n", 
                  NUM_CONFIGS, frame_count, burst_period);

    // Wait for user to confirm they're ready before starting
    Serial.println("#");
    Serial.println("# ==========================================");
    Serial.println("# Position your sensors, then press ENTER to start sampling...");
    Serial.println("# ==========================================");
    
    // Wait indefinitely for user input
    while (!Serial.available()) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Clear the input buffer
    while (Serial.available()) {
        Serial.read();
    }
    
    Serial.println("# Starting FTM sampling...");
    Serial.println("#");

    // Send initial FTM request to trigger the first measurement
    ftm_request_start_time = millis();
    puara.requestFTM();
}


void loop() {
    
    // Check if all distances are complete
    if (all_distances_complete) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }
    
    // Wait for user confirmation to proceed to next distance
    if (waiting_for_user) {
        if (Serial.available()) {
            // Read and discard input (any key press)
            while (Serial.available()) {
                Serial.read();
            }
            // Advance to next distance
            advanceToNextDistance();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
        return;
    }

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