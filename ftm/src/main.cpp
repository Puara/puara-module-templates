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
#include "ftm_utils.h"

#include <iostream>

Puara puara;

// Default FTM configuration to be updated by values in settings.json in setup()
uint8_t frame_count;
uint16_t burst_period;
int16_t manual_ftm_offset;

// Timing measurement variables
unsigned long ftm_request_start_time = 0;

// Moving average filter for distance smoothing (window size = 5)
MovingAverage distanceFilter(5);

// Startup calibration (100cm known distance, 100 samples)
FTMCalibration calibrator(100.0f, 100);

void onSettingsChanged() {
  puara.end_ftm_request_session(); // End ongoing FTM session to apply new settings immediately
  
  // Discard any pending report from the old configuration
  if (puara.is_ftm_report_available()) {
    puara.set_ftm_report_as_consumed();
  }
  
  // Reset the moving average filter since config changed
  distanceFilter.reset();

  frame_count = puara.getVarNumber("frame count");
  burst_period = puara.getVarNumber("burst period");
  puara.configureFTM(frame_count, burst_period); 
  
  Serial.printf("Settings updated - frames: %u, burst: %u\n", frame_count, burst_period);
  
  ftm_request_start_time = millis();
  puara.requestFTM(); // Trigger new FTM procedure with updated settings
}



void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    //puara.start(PuaraAPI::UART_MONITOR, ESP_LOG_VERBOSE);
    puara.start();
    puara.set_settings_changed_handler(onSettingsChanged);
    frame_count = puara.getVarNumber("frame count");
    burst_period = puara.getVarNumber("burst period");
    //manual_ftm_offset = puara.getVarNumber("manual FTM offset");

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
        Serial.println("External Access Point is not an FTM responder. Please connect to an FTM responder to perform ranging.");
    }

    // Startup calibration instructions
    Serial.println("\n===== FTM CALIBRATION =====");
    Serial.printf("Hold device at %.0f cm from responder\n", calibrator.getKnownDistance());
    Serial.printf("Collecting %u samples...\n", calibrator.getTargetSamples());
    Serial.println("===========================\n");

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
        
        // CALIBRATION PHASE: Collect samples at known distance
        if (calibrator.isCalibrating()) {
            bool just_finished = calibrator.addSample((float)distance_cm);
            
            // Print progress every 10 samples
            if (calibrator.getSamplesCollected() % 10 == 0) {
                Serial.printf("Calibration: %u/%u samples (%.1f cm avg)\n",
                              calibrator.getSamplesCollected(),
                              calibrator.getTargetSamples(),
                              calibrator.getMeasuredAverage());
            }
            
            if (just_finished) {
                Serial.println("===== CALIBRATION COMPLETE =====");
                Serial.printf("Known distance: %.1f cm\n", calibrator.getKnownDistance());
                Serial.printf("Measured average: %.1f cm\n", calibrator.getMeasuredAverage());
                Serial.printf("Calculated offset: %.1f cm\n", calibrator.getOffset());
                Serial.println("================================");
            }
            
            puara.set_ftm_report_as_consumed();
            ftm_request_start_time = millis();
            puara.requestFTM();
            return;  // Skip normal processing during calibration
        }
        
        // NORMAL OPERATION: Apply calibration and filter
        float calibrated_distance = calibrator.apply((float)distance_cm);
        float filtered_distance = distanceFilter.addSample(calibrated_distance);
        
        Serial.printf("FTM Report - Raw: %u cm, Calibrated: %.1f cm, Filtered: %.1f cm, RTT: %u ns, Time: %s\n", 
                      distance_cm, calibrated_distance, filtered_distance, rtt_ns, time_buffer);
        
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