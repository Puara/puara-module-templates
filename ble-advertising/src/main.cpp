//****************************************************************************//
// Puara Module Manager                                                       //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
//****************************************************************************//

#include "Arduino.h"

// nimBLE-Arduino library for BLE functionality
#include "NimBLEDevice.h"
#include "MicroCbor.hpp"

// Include Puara's module manager
// If using Arduino.h, include it before including puara.h
#include "puara.h"

#include <iostream>

// Initialize Puara's module manager
Puara puara;

// dummy sensor data
int32_t sensor1;
int32_t sensor2;

void setup() {
    #ifdef Arduino_h
        Serial.begin(115200);
    #endif
    // Initialize NimBLE device
    NimBLEDevice::init(puara.dmi_name());

    /*
     * The Puara start function initializes the spiffs, reads the config and custom JSON
     * settings, start the wi-fi AP, connects to SSID, starts the webserver, serial
     * listening, MDNS service, and scans for WiFi networks.
     */
    puara.start();

    // Printing custom settings stored:
    std::cout << "\n"
    << "Settings stored in settings.json:\n"
    << "Hitchhiker: " << puara.getVarText ("Hitchhiker") << "\n"
    << "answer_to_everything: " << puara.getVarNumber("answer_to_everything")
    << "\n" << std::endl;

    std::cout<< "This template must be used with the BLE-CBOR-to-OSC_script.\n"
    << "It advertises dummy sensor data as CBOR over BLE Manufacturer Data\n"
    << "packets. The script scans for the advertisement, decodes the CBOR data,\n"
    << "and sends it as OSC messages to be used in your favorite environment.\n"
    << std::endl;

}

constexpr std::array<uint8_t, 2> manufacturer_id = {0xFF, 0xFF};

float target_frequency = 50.0;

// the ble advertising interval value to set for a given target frequency is always
// 1/f/0.000625. BLE advertising intervals are quantized in steps of 0.625ms
uint16_t ble_interval_value = static_cast<uint16_t>((1/target_frequency)/0.000625);

uint16_t period_ms = static_cast<uint16_t>((1/target_frequency) * 1000);

// allocate a 32-byte vector for the BLE advertisement bytes.
std::vector<uint8_t> advert_data(32);

void loop() {
    // Update dummy sensor with random number
    sensor1 = static_cast <int32_t>(rand());
    sensor2 = static_cast <int32_t>(rand());

    // Set the new values in the data CBOR object
    advert_data.resize(32);
    entazza::MicroCbor cbor(advert_data.data(), advert_data.size());
    cbor.startMap();
    cbor.add("sensor1", sensor1);
    cbor.add("sensor2", sensor2);
    cbor.endMap();

    advert_data.resize(cbor.bytesSerialized());

    // We can only have 27 bytes of real payload. A legacy BLE advertising packet is 31 bytes.
    // 2 of those are used to indicate that we are sending a manufacturer data packet.
    // 2 others need to be the Bluetooth manufacturer ID.
    if (advert_data.size() > 27) {
      Serial.println("too much data for BLE advertising");
      return;
    }

    // Set up the advertisement with NimBLE
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    
    // Prepend manufacturer ID to the CBOR data
    std::vector<uint8_t> ble_data;
    ble_data.push_back(manufacturer_id[0]);
    ble_data.push_back(manufacturer_id[1]);
    ble_data.insert(ble_data.end(), advert_data.begin(), advert_data.end());
    
    // Create advertisement data with manufacturer data
    NimBLEAdvertisementData advertisementData;
    advertisementData.setManufacturerData(ble_data);
    
    // Configure scan response data with device name
    NimBLEAdvertisementData scanResponseData;
    scanResponseData.setName(puara.dmi_name());
    
    // Apply the advertising configuration
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->setScanResponseData(scanResponseData);
    
    // Set advertising interval (valid range: 20ms to 10.24s in 0.625ms steps)
    // 32 is 20ms and 16384 is 10.24s
    pAdvertising->setMinInterval(ble_interval_value);
    pAdvertising->setMaxInterval(ble_interval_value);
    
    // Start advertising
    pAdvertising->start();
    
    // run at the target frequency
    vTaskDelay(period_ms / portTICK_PERIOD_MS);
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
