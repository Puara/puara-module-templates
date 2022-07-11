//****************************************************************************//
// Puara Module Manager - WiFi and file system functions                      //
// Metalab - Société des Arts Technologiques (SAT)                            //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Edu Meneses (2022) - https://www.edumeneses.com                            //
//****************************************************************************//

#ifndef PUARA_H
#define PUARA_H

#include <stdio.h>
#include <string>
#include <cstring>
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_err.h>
#include <esp_spiffs.h>
#include <cJSON.h>
#include <esp_http_server.h>
#include <driver/uart.h>
#include <mdns.h>

// The following libraries need to be included if using the espidf framework:

// #include <esp_log.h>
// #include <sys/unistd.h>
// #include <sys/stat.h>
// #include <lwip/err.h>
// #include <lwip/sys.h>
// #include <esp_event.h>
// #include <soc/uart_struct.h>

// The following library requires arduino framework to compile
#include <mapper_cpp.h>   // libmapper

class Puara {
    
    private:
        static const unsigned int VERSION = 220711;
        static std::string dmiName;

        struct settingsVariables {
            std::string name;
            std::string type;
            std::string textValue;
            double numberValue;
        };
        
        static std::vector<settingsVariables> variables;
        static std::unordered_map<std::string,int> variables_fields;

        static lo_address lo_oscIP1;
        static lo_address lo_oscIP2;

        static std::unordered_map<std::string,int> config_fields;
        static std::string device;
        static unsigned int id;
        static std::string author;
        static std::string institution;
        static std::string APpasswd;
        static std::string APpasswdVal1;
        static std::string APpasswdVal2;
        static std::string wifiSSID;
        static std::string wifiPSK;
        static bool persistentAP;
        static std::string oscIP1;
        static unsigned int oscPORT1;
        static std::string oscIP2;
        static unsigned int oscPORT2;
        static unsigned int localPORT;
        
        static bool StaIsConnected;
        static bool ApStarted;
        static std::string currentSSID;
        static std::string currentSTA_IP;
        static std::string currentSTA_MAC;
        static std::string currentAP_IP;
        static std::string currentAP_MAC;
        static const int wifiScanSize = 20;
        static std::string wifiAvailableSsid;

        static EventGroupHandle_t s_wifi_event_group; // FreeRTOS event group to 
                                                      // signal when we are connected
        static const int wifi_connected_bit = BIT0;   // we are connected to the AP 
                                                      // with an IP
        static const int wifi_fail_bit = BIT1;        // fails to connect after the 
                                                      // maximum amount of retries
        static wifi_config_t wifi_config_sta;
        static wifi_config_t wifi_config_ap;
        static const short int channel = 6;
        static const short int max_connection = 5;
        static const short int wifi_maximum_retry = 5;
        static short int connect_counter;
        static void sta_event_handler(void* arg, esp_event_base_t event_base, int event_id, void* event_data);
        static void ap_event_handler(void* arg, esp_event_base_t event_base, int event_id, void* event_data);
        static void wifi_init();

        static httpd_handle_t webserver;
        static httpd_config_t webserver_config;
        static httpd_uri_t index;
        static httpd_uri_t style;
        static httpd_uri_t factory;
        static httpd_uri_t reboot;
        static httpd_uri_t scan;
        // static httpd_uri_t update;
        static httpd_uri_t indexpost;
        static httpd_uri_t settings;
        static httpd_uri_t settingspost;
        static esp_err_t index_get_handler(httpd_req_t *req);
        static esp_err_t get_handler(httpd_req_t *req);
        static esp_err_t style_get_handler(httpd_req_t *req);
        static esp_err_t settings_get_handler(httpd_req_t *req);
        static esp_err_t settings_post_handler(httpd_req_t *req);
        static esp_err_t scan_get_handler(httpd_req_t *req);
        static esp_err_t index_post_handler(httpd_req_t *req);
        static std::string prepare_index();
        static void find_and_replace(std::string old_text, std::string new_text, std::string &str);
        static void find_and_replace(std::string old_text, double new_number, std::string &str);
        static void find_and_replace(std::string old_text, unsigned int new_number, std::string &str);
        static void checkmark(std::string old_text, bool value, std::string & str);
        static esp_vfs_spiffs_conf_t spiffs_config;
        static std::string spiffs_base_path;
        static const uint8_t spiffs_max_files = 10;
        static const bool spiffs_format_if_mount_failed = false;

        static char serial_data[12];
        static int serial_data_length;
        static std::string serial_data_str;
        static std::string convertToString(char* a);
        static void interpret_serial(void *pvParameter);
        static void serial_monitor(void *pvParameters);
        static const int reboot_delay = 3000;
        static void reboot_with_delay(void *pvParameter);
        static std::string urlDecode(std::string text);
    
    public:
        static void start(); 
        static void config_spiffs();
        static httpd_handle_t start_webserver(void);
        static void stop_webserver(void);
        static void start_wifi();
        std::string get_dmi_name();
        static int get_version();
        static void mount_spiffs();
        static void unmount_spiffs();
        static void read_config_json();
        static void write_config_json();
        static void read_settings_json();
        static void write_settings_json();
        static bool start_serial_listening();
        static void start_mdns_service(const char * device_name, const char * instance_name);
        static void start_mdns_service(std::string device_name, std::string instance_name);
        static void wifi_scan(void);
        static double getVarNumber (std::string varName);
        static std::string getVarText(std::string varName);
        static void start_communication();
        void createMessage(std::string name, int length, std::string unit, float mina, float maxa, std::string mode);
        void setMessage(std::string name, float value);

        static mapper::Device* lmDev;
        static std::vector<mapper::Signal> lmSignals;
        static std::vector<float> lmMin;
        static std::vector<float> lmMax;
        static std::unordered_map<std::string,int> lm_fields;
};

#endif