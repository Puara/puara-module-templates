//****************************************************************************//
// Puara FTM Benchmark Test                                                   //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL)                     //
//                                                                            //
//   FTM distance measurement benchmark.                                      //
//   Collects 1000 samples per responder at a single physical position,       //
//   outputs CSV-ready data over serial.                                      //
//                                                                            //
//   Change CURRENT_POSITION below and reflash for each test position.        //
//                                                                            //
//   Responder layout:               Test position grid:                      //
//     r1(0,0)    r2(20,0)             p11(5,3)  p12(10,3)  p13(15,3)        //
//     r3(0,15)   r4(20,15)            p21(5,6)  p22(10,6)  p23(15,6)        //
//                                     p31(5,9)  p32(10,9)  p33(15,9)        //
//****************************************************************************//

#include "Arduino.h"
#include "puara.h"

#include <iostream>
#include <map>
#include <vector>
#include <string>

// ======================== TEST CONFIGURATION ================================
// Select which position you are testing by changing CURRENT_POSITION.
// Reflash for each new position.

#define POS_P11 0
#define POS_P12 1
#define POS_P13 2
#define POS_P21 3
#define POS_P22 4
#define POS_P23 5
#define POS_P31 6
#define POS_P32 7
#define POS_P33 8

#define CURRENT_POSITION POS_P11   // <── CHANGE THIS FOR EACH TEST

#define SAMPLES_PER_RESPONDER 1000 // 1000 attempts per responder
#define FTM_FRAME_COUNT       16
#define FTM_BURST_PERIOD      4
#define MLAT_MODE             4    // 4 responders
#define FTM_TIMEOUT_MS        1000

// ============================================================================

// ── Position & responder metadata ──────────────────────────────────────────

struct TestPosition {
    const char* label;
    float x;
    float y;
};

static const TestPosition test_positions[] = {
    {"p11",  5.0f,  3.0f},   // POS_P11
    {"p12", 10.0f,  3.0f},   // POS_P12
    {"p13", 15.0f,  3.0f},   // POS_P13
    {"p21",  5.0f,  6.0f},   // POS_P21
    {"p22", 10.0f,  6.0f},   // POS_P22
    {"p23", 15.0f,  6.0f},   // POS_P23
    {"p31",  5.0f,  9.0f},   // POS_P31
    {"p32", 10.0f,  9.0f},   // POS_P32
    {"p33", 15.0f,  9.0f},   // POS_P33
};

struct ResponderInfo {
    const char* label;
    float x;
    float y;
};

static const ResponderInfo responder_info[] = {
    {"r1",  0.0f,  0.0f},
    {"r2", 20.0f,  0.0f},
    {"r3",  0.0f, 15.0f},
    {"r4", 20.0f, 15.0f},
};

// ── Globals ────────────────────────────────────────────────────────────────

Puara puara;

uint8_t  frame_count  = FTM_FRAME_COUNT;
uint16_t burst_period = FTM_BURST_PERIOD;

std::map<std::string, wifi_ftm_initiator_cfg_t> responder_configs;

// Ordered list of (SSID, responder-index 0-3) so iteration order is stable
std::vector<std::pair<std::string, int>> ordered_responders;

// Per-responder counters
int err_count_r1 = 0;
int err_count_r2 = 0;
int err_count_r3 = 0;
int err_count_r4 = 0;
int* err_count[]     = {&err_count_r1, &err_count_r2, &err_count_r3, &err_count_r4};
int  success_count[] = {0, 0, 0, 0};

int  current_round   = 0;
bool test_complete   = false;
bool summary_printed = false;

// ── Forward declarations ───────────────────────────────────────────────────

void init_FTM(int mlat_mode);

// ── setup() ────────────────────────────────────────────────────────────────

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif

    puara.start();
    init_FTM(MLAT_MODE);

    const TestPosition& pos = test_positions[CURRENT_POSITION];

    // Metadata as comments (prefix with # so CSV parsers can skip them)
    Serial.printf("# ====== FTM Benchmark Test ======\n");
    Serial.printf("# Position       : %s (%.1f, %.1f)\n", pos.label, pos.x, pos.y);
    Serial.printf("# Responders     : %d\n", (int)ordered_responders.size());
    Serial.printf("# Samples/resp   : %d\n", SAMPLES_PER_RESPONDER);
    Serial.printf("# Total attempts : %d\n", (int)ordered_responders.size() * SAMPLES_PER_RESPONDER);
    Serial.printf("# Frame count    : %d\n", FTM_FRAME_COUNT);
    Serial.printf("# Burst period   : %d\n", FTM_BURST_PERIOD);
    for (int i = 0; i < (int)ordered_responders.size(); i++) {
        int ri = ordered_responders[i].second;
        Serial.printf("# Responder %s : SSID=%s  pos=(%.1f, %.1f)\n",
            responder_info[ri].label,
            ordered_responders[i].first.c_str(),
            responder_info[ri].x, responder_info[ri].y);
    }
    Serial.printf("# ================================\n");

    // CSV header
    Serial.println("position,real_x,real_y,responder_ssid,responder_label,responder_x,responder_y,frame_count,burst_period,dist_est_cm,rtt_est_ns,rssi,elapsed_ms,sample_num");
}

// ── loop() ─────────────────────────────────────────────────────────────────

void loop() {
    // ── Test finished: print summary once, then idle ──
    if (test_complete) {
        if (!summary_printed) {
            summary_printed = true;

            const TestPosition& pos = test_positions[CURRENT_POSITION];
            int total_ok  = 0;
            int total_err = 0;

            Serial.printf("# ====== TEST COMPLETE ======\n");
            Serial.printf("# Position: %s (%.1f, %.1f)\n", pos.label, pos.x, pos.y);
            for (int i = 0; i < 4; i++) {
                Serial.printf("# %s (%s): %d successes, %d errors\n",
                    responder_info[i].label,
                    (i < (int)ordered_responders.size()) ? ordered_responders[i].first.c_str() : "N/A",
                    success_count[i], *err_count[i]);
                total_ok  += success_count[i];
                total_err += *err_count[i];
            }
            Serial.printf("# Total: %d successes, %d errors out of %d attempts\n",
                total_ok, total_err, total_ok + total_err);
            Serial.printf("# ===========================\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }

    // ── Check if all rounds are done ──
    if (current_round >= SAMPLES_PER_RESPONDER) {
        test_complete = true;
        return;
    }

    const TestPosition& pos = test_positions[CURRENT_POSITION];

    // ── One round: query each responder once ──
    for (auto& [ssid, resp_idx] : ordered_responders) {
        auto cfg_it = responder_configs.find(ssid);
        if (cfg_it == responder_configs.end()) continue;

        unsigned long ftm_start = millis();
        esp_wifi_ftm_initiate_session(&(cfg_it->second));

        // Wait for report or timeout
        while (!puara.ftm_report_available()) {
            vTaskDelay(1 / portTICK_PERIOD_MS);
            if (millis() - ftm_start > FTM_TIMEOUT_MS) {
                break;
            }
        }

        if (puara.ftm_report_available()) {
            unsigned long elapsed_ms = millis() - ftm_start;
            PuaraAPI::FTM::ftm_report_info report = puara.get_last_ftm_report();
            puara.set_ftm_report_as_consumed();

            // CSV data line
            Serial.printf("%s,%.1f,%.1f,%s,%s,%.1f,%.1f,%d,%d,%u,%u,%d,%lu,%d\n",
                pos.label, pos.x, pos.y,
                ssid.c_str(),
                responder_info[resp_idx].label,
                responder_info[resp_idx].x, responder_info[resp_idx].y,
                FTM_FRAME_COUNT, FTM_BURST_PERIOD,
                report.dist_est, report.rtt_est, report.rssi,
                elapsed_ms,
                current_round + 1);

            success_count[resp_idx]++;
        } else {
            // Timeout or FTM failure
            (*err_count[resp_idx])++;
            Serial.printf("# ERROR round %d: %s (%s) — timeout/failure\n",
                current_round + 1, ssid.c_str(), responder_info[resp_idx].label);
            puara.set_ftm_report_as_consumed();  // clear any stale state
        }
    }

    current_round++;

    // Progress every 100 rounds
    if (current_round % 100 == 0) {
        Serial.printf("# Progress: %d / %d rounds complete\n", current_round, SAMPLES_PER_RESPONDER);
    }
}

// ── init_FTM() ─────────────────────────────────────────────────────────────

void init_FTM(int mlat_mode) {
    std::vector<std::string> ssid_list;

    for (int i = 1; i <= mlat_mode; ++i) {
        std::string key = "Responder" + std::to_string(i) + "_SSID";
        std::string ssid = puara.getVarText(key);
        if (!ssid.empty()) {
            Serial.printf("Retrieved SSID for Responder %d: %s\n", i, ssid.c_str());
            ssid_list.push_back(ssid);
            ordered_responders.push_back({ssid, i - 1});  // 0-indexed responder
        }
    }

    responder_configs = puara.get_map_of_responder_configs(frame_count, burst_period, ssid_list);
}

// ── ESP-IDF entry point (non-Arduino builds) ──────────────────────────────

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