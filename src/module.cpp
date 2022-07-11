/* 

Puara Module Manager                                                     
Metalab - Société des Arts Technologiques (SAT)                          
Input Devices and Music Interaction Laboratory (IDMIL), McGill University
Edu Meneses (2022) - https://www.edumeneses.com                          

- event_handler, wifi_init_sta, and start_wifi were modified from 
  https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station
- mount_spiffs, and unmount_spiffs were modified from
  https://github.com/espressif/esp-idf/tree/master/examples/storage

*/

#include <module.h>

// Defining static members
std::string Puara::dmiName;
std::string Puara::device;
unsigned int Puara::id;
std::string Puara::author;
std::string Puara::institution;
std::string Puara::APpasswd;
std::string Puara::APpasswdVal1;
std::string Puara::APpasswdVal2;
std::string Puara::wifiSSID;
std::string Puara::wifiPSK;
bool Puara::persistentAP = false;
std::string Puara::oscIP1;
unsigned int Puara::oscPORT1;
std::string Puara::oscIP2;
unsigned int Puara::oscPORT2;
unsigned int Puara::localPORT;
std::string Puara::wifiAvailableSsid;
std::string Puara::currentSSID;

std::unordered_map<std::string,int> Puara::config_fields = {
    {"SSID",1},
    {"APpasswd",2},
    {"APpasswdValidate",3},
    {"oscIP1",4},
    {"oscPORT1",5},
    {"oscIP2",6},
    {"oscPORT2",7},
    {"password",8},
    {"reboot",9},
    {"persistentAP",10},
    {"localPORT",11}
};

std::vector<Puara::settingsVariables> Puara::variables;
std::unordered_map<std::string,int> Puara::variables_fields;

mapper::Device* Puara::lmDev;
std::vector<mapper::Signal> Puara::lmSignals;
std::unordered_map<std::string,int> Puara::lm_fields;
std::vector<float> Puara::lmMin;
std::vector<float> Puara::lmMax;

lo_address Puara::lo_oscIP1;
lo_address Puara::lo_oscIP2;

std::string Puara::currentSTA_IP;
std::string Puara::currentSTA_MAC;
std::string Puara::currentAP_IP;
std::string Puara::currentAP_MAC;
bool Puara::StaIsConnected = false;
bool Puara::ApStarted = false;

esp_vfs_spiffs_conf_t Puara::spiffs_config;
std::string Puara::spiffs_base_path;
EventGroupHandle_t Puara::s_wifi_event_group;
wifi_config_t Puara::wifi_config_sta;
wifi_config_t Puara::wifi_config_ap;
short int Puara::connect_counter;
httpd_handle_t Puara::webserver;
httpd_config_t Puara::webserver_config;
httpd_uri_t Puara::index;
httpd_uri_t Puara::style;
httpd_uri_t Puara::factory;
httpd_uri_t Puara::reboot;
httpd_uri_t Puara::scan;
//httpd_uri_t Puara::update;
httpd_uri_t Puara::indexpost;
httpd_uri_t Puara::settings;
httpd_uri_t Puara::settingspost;

char Puara::serial_data[12];
int Puara::serial_data_length;
std::string Puara::serial_data_str;

int Puara::get_version() {
    return VERSION;
};

void Puara::start() {
    printf("\n");
    printf("**********************************************************\n");
    printf("* Puara Module Manager                                   *\n");
    printf("* Metalab - Société des Arts Technologiques (SAT)        *\n");
    printf("* Input Devices and Music Interaction Laboratory (IDMIL) *\n");
    printf("* Edu Meneses (2022) - https://www.edumeneses.com        *\n");
    printf("* Firmware version %d                                *\n", VERSION);                 
    printf("**********************************************************\n\n");
    
    config_spiffs();    
    read_config_json();
    read_settings_json();
    start_wifi();
    start_webserver();
    start_serial_listening();
    start_mdns_service(dmiName, dmiName);
    start_communication();
    wifi_scan();

    printf("Done!\n");
}

void Puara::sta_event_handler(void* arg, esp_event_base_t event_base, 
                               int event_id, void* event_data) {
    //int counter = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && 
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("%d, %d", Puara::connect_counter, Puara::wifi_maximum_retry);
        if (Puara::connect_counter < Puara::wifi_maximum_retry) {
            Puara::connect_counter++;
            esp_wifi_connect();
            std::cout << "wifi/sta_event_handler: retry to connect to the AP" << "\n";
        } else {
            xEventGroupSetBits(s_wifi_event_group, Puara::wifi_fail_bit);
        }
        std::cout << "wifi/sta_event_handler: connect to the AP fail" << "\n";
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        std::stringstream tempBuf;
        tempBuf << esp_ip4_addr1_16(&event->ip_info.ip) << ".";
        tempBuf << esp_ip4_addr2_16(&event->ip_info.ip) << ".";
        tempBuf << esp_ip4_addr3_16(&event->ip_info.ip) << ".";
        tempBuf << esp_ip4_addr4_16(&event->ip_info.ip);
        Puara::currentSTA_IP = tempBuf.str();
        std::cout << "wifi/sta_event_handler: got ip:" << Puara::currentSTA_IP << "\n";
        Puara::connect_counter = 0;
        xEventGroupSetBits(s_wifi_event_group, Puara::wifi_connected_bit);
    }
}

void Puara::wifi_init() {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap(); // saving pointer to 
                                                                // retrieve AP ip later

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Set device hostname
    esp_err_t setname = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, 
                                                  dmiName.c_str());
    if(setname != ESP_OK ){
        std::cout << "wifi_init: failed to set hostname: " << dmiName  << "\n";  
    } else {
        std::cout << "wifi_init: hostname: " << dmiName << "\n";  
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &Puara::sta_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &Puara::sta_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    std::cout << "wifi_init: setting wifi mode" << "\n";
    if (persistentAP) {
        std::cout << "wifi_init:     AP-STA mode" << "\n";
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        std::cout << "wifi_init: loading AP config" << "\n";
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    } else {
        std::cout << "wifi_init:     STA mode" << "\n";
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    }
    std::cout << "wifi_init: loading STA config" << "\n";
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta) );
    std::cout << "wifi_init: esp_wifi_start" << "\n";
    ESP_ERROR_CHECK(esp_wifi_start());

    std::cout << "wifi_init: wifi_init finished." << "\n";

    /* Waiting until either the connection is established (Puara::wifi_connected_bit)
     * or connection failed for the maximum number of re-tries (Puara::wifi_fail_bit).
     * The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            Puara::wifi_connected_bit | Puara::wifi_fail_bit,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we
     * can test which event actually happened. */
    if (bits & Puara::wifi_connected_bit) {
        std::cout << "wifi_init: Connected to SSID: " << Puara::wifiSSID  << "\n";
        currentSSID = wifiSSID;
        Puara::StaIsConnected = true;
    } else if (bits & Puara::wifi_fail_bit) {
        std::cout << "wifi_init: Failed to connect to SSID: " << Puara::wifiSSID  << "\n";
        if (!persistentAP) {
            std::cout << "wifi_init: Failed to connect to SSID: " << Puara::wifiSSID << "Switching to AP/STA mode" << "\n";
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
            std::cout << "wifi_init: loading AP config" << "\n";
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
            std::cout << "wifi_init: Trying to connect one more time to SSID before giving up." << "\n";
            ESP_ERROR_CHECK(esp_wifi_start());
        } else {
            Puara::StaIsConnected = false;
        }
    } else {
        std::cout << "wifi_init: UNEXPECTED EVENT" << "\n";
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, 
                                                          IP_EVENT_STA_GOT_IP, 
                                                          instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, 
                                                          ESP_EVENT_ANY_ID, 
                                                          instance_any_id));
    vEventGroupDelete(s_wifi_event_group);

    // getting extra info
    unsigned char temp_info[6] = {0};
    esp_wifi_get_mac(WIFI_IF_STA, temp_info);
    std::ostringstream tempBuf;
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[0] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[1] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[2] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[3] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[4] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[5];
    Puara::currentSTA_MAC = tempBuf.str();
    tempBuf.clear();            // preparing the ostringstream 
    tempBuf.str(std::string()); // buffer for reuse
    esp_wifi_get_mac(WIFI_IF_AP, temp_info);
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[0] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[1] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[2] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[3] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[4] << ":";
    tempBuf << std::setfill('0') << std::setw(2) << std::hex << (int)temp_info[5];
    Puara::currentAP_MAC = tempBuf.str();

    esp_netif_ip_info_t ip_temp_info;
    esp_netif_get_ip_info(ap_netif, &ip_temp_info);
    tempBuf.clear();
    tempBuf.str(std::string());
    tempBuf << std::dec << esp_ip4_addr1_16(&ip_temp_info.ip) << ".";
    tempBuf << std::dec << esp_ip4_addr2_16(&ip_temp_info.ip) << ".";
    tempBuf << std::dec << esp_ip4_addr3_16(&ip_temp_info.ip) << ".";
    tempBuf << std::dec << esp_ip4_addr4_16(&ip_temp_info.ip);
    Puara::currentAP_IP = tempBuf.str();
}

void Puara::start_wifi() {

    ApStarted = false;

    // Check if wifiSSID is empty and wifiPSK have less than 8 characteres
    if (dmiName.empty() ) {
        std::cout << "start_wifi: Module name unpopulated. Using default name: Puara" << "\n";
       dmiName = "Puara";
    }
    if ( APpasswd.empty() || APpasswd.length() < 8 || APpasswd == "password" ) {
        std::cout << "startWifi: AP password error. Possible causes:" << "\n";
        std::cout << "startWifi:   - no AP password" << "\n";
        std::cout << "startWifi:   - password is less than 8 characteres long" << "\n";
        std::cout << "startWifi:   - password is set to \"password\"" << "\n";
        std::cout << "startWifi: Using default AP password: password" << "\n";
        std::cout << "startWifi: It is strongly recommended to change the password" << "\n";
        APpasswd = "password";
    }
    if ( wifiSSID.empty() ) {
        std::cout << "start_wifi: No blank SSID allowed. Using default name: Puara" << "\n";
        wifiSSID = "Puara";
    }

    strncpy((char *) Puara::wifi_config_sta.sta.ssid, Puara::wifiSSID.c_str(),
            Puara::wifiSSID.length() + 1);
    strncpy((char *) Puara::wifi_config_sta.sta.password, Puara::wifiPSK.c_str(),
            Puara::wifiPSK.length() + 1);
    strncpy((char *) Puara::wifi_config_ap.ap.ssid, Puara::dmiName.c_str(),
            Puara::dmiName.length() + 1);
    Puara::wifi_config_ap.ap.ssid_len = Puara::dmiName.length();
    Puara::wifi_config_ap.ap.channel = Puara::channel;
    strncpy((char *) Puara::wifi_config_ap.ap.password, Puara::APpasswd.c_str(), 
            Puara::APpasswd.length() + 1);
    Puara::wifi_config_ap.ap.max_connection = Puara::max_connection;
    Puara::wifi_config_ap.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    //Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

    std::cout << "startWifi: Starting WiFi config" << "\n";
    Puara::connect_counter = 0;
    wifi_init();
    ApStarted = true;
}

void Puara::config_spiffs() {
    spiffs_base_path = "/spiffs";
}

void Puara::mount_spiffs() {

    if (!esp_spiffs_mounted(spiffs_config.partition_label)) {
        std::cout << "spiffs: Initializing SPIFFS" << "\n";

        spiffs_config.base_path = Puara::spiffs_base_path.c_str();
        spiffs_config.max_files = Puara::spiffs_max_files;
        spiffs_config.partition_label = NULL;
        spiffs_config.format_if_mount_failed = Puara::spiffs_format_if_mount_failed;

        // Use settings defined above to initialize and mount SPIFFS filesystem.
        // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
        esp_err_t ret = esp_vfs_spiffs_register(&spiffs_config);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                std::cout << "spiffs: Failed to mount or format filesystem" << "\n";
            } else if (ret == ESP_ERR_NOT_FOUND) {
                std::cout << "spiffs: Failed to find SPIFFS partition" << "\n";
            } else {
                std::cout << "spiffs: Failed to initialize SPIFFS (" << esp_err_to_name(ret) << ")"  << "\n";
            }
            return;
        }

        size_t total = 0, used = 0;
        ret = esp_spiffs_info(spiffs_config.partition_label, &total, &used);
        if (ret != ESP_OK) {
            std::cout << "spiffs: Failed to get SPIFFS partition information (" << esp_err_to_name(ret) << ")"  << "\n";
        } else {
            std::cout << "spiffs: Partition size: total: " << total << ", used: " << used  << "\n";
        }
    } else {
        std::cout << "spiffs: SPIFFS already initialized" << "\n";
    }
}

void Puara::unmount_spiffs() {
    // All done, unmount partition and disable SPIFFS
    if (esp_spiffs_mounted(spiffs_config.partition_label)) {
        esp_vfs_spiffs_unregister(spiffs_config.partition_label);
        std::cout << "spiffs: SPIFFS unmounted" << "\n";
    } else {
        std::cout << "spiffs: SPIFFS not found" << "\n";
    }
}

void Puara::read_config_json() { // Deserialize
    
    std::cout << "json: Mounting FS" << "\n";
    Puara::mount_spiffs();

    std::cout << "json: Opening config json file" << "\n";
    FILE* f = fopen("/spiffs/config.json", "r");
    if (f == NULL) {
        std::cout << "json: Failed to open file" << "\n";
        return;
    }

    std::cout << "json: Reading json file" << "\n";
    std::ifstream in("/spiffs/config.json");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());

    std::cout << "json: Getting data" << "\n";
    cJSON *root = cJSON_Parse(contents.c_str());
    if (cJSON_GetObjectItem(root, "device")) {
        Puara::device = cJSON_GetObjectItem(root,"device")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "id")) {
        Puara::id = cJSON_GetObjectItem(root,"id")->valueint;
    }
    if (cJSON_GetObjectItem(root, "author")) {
        Puara::author = cJSON_GetObjectItem(root,"author")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "institution")) {
        Puara::institution = cJSON_GetObjectItem(root,"institution")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "APpasswd")) {
        Puara::APpasswd = cJSON_GetObjectItem(root,"APpasswd")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "wifiSSID")) {
        Puara::wifiSSID = cJSON_GetObjectItem(root,"wifiSSID")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "wifiPSK")) {
        Puara::wifiPSK = cJSON_GetObjectItem(root,"wifiPSK")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "persistentAP")) {
        Puara::persistentAP = cJSON_GetObjectItem(root,"persistentAP")->valueint;
    }
    if (cJSON_GetObjectItem(root, "oscIP1")) {
        Puara::oscIP1 = cJSON_GetObjectItem(root,"oscIP1")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "oscPORT1")) {
        Puara::oscPORT1 = cJSON_GetObjectItem(root,"oscPORT1")->valueint;
    }
    if (cJSON_GetObjectItem(root, "oscIP2")) {
        Puara::oscIP2 = cJSON_GetObjectItem(root,"oscIP2")->valuestring;
    }
    if (cJSON_GetObjectItem(root, "oscPORT2")) {
        Puara::oscPORT2 = cJSON_GetObjectItem(root,"oscPORT2")->valueint;
    }
    if (cJSON_GetObjectItem(root, "localPORT")) {
        Puara::localPORT = cJSON_GetObjectItem(root,"localPORT")->valueint;
    }
    
    std::cout << "json: Data collected:" << "\n";
    std::cout << "\ndevice: " << device << "\n";
    std::cout << "id: " << id << "\n";
    std::cout << "author: " << author << "\n";
    std::cout << "institution: " << institution << "\n";
    std::cout << "APpasswd: " << APpasswd << "\n";
    std::cout << "wifiSSID: " << wifiSSID << "\n";
    std::cout << "wifiPSK: " << wifiPSK << "\n";
    std::cout << "persistentAP: " << persistentAP << "\n";
    std::cout << "oscIP1: " << oscIP1 << "\n";
    std::cout << "oscPORT1: " << oscPORT1 << "\n";
    std::cout << "oscIP2: " << oscIP2 << "\n";
    std::cout << "oscPORT2: " << oscPORT2 << "\n";
    std::cout << "localPORT: " << localPORT << "\n\n";
    
    cJSON_Delete(root);

    std::stringstream tempBuf;
    tempBuf << Puara::device << "_" << std::setfill('0') << std::setw(3) << Puara::id;
    Puara::dmiName = tempBuf.str();
    printf("Device unique name defined: %s\n",dmiName.c_str());

    Puara::lo_oscIP1 = lo_address_new(oscIP1.c_str(), std::to_string(oscPORT1).c_str());
    Puara::lo_oscIP2 = lo_address_new(oscIP2.c_str(), std::to_string(oscPORT2).c_str());

    fclose(f);
    Puara::unmount_spiffs();
}

void Puara::read_settings_json() {

    std::cout << "json: Mounting FS" << "\n";
    Puara::mount_spiffs();

    std::cout << "json: Opening settings json file" << "\n";
    FILE* f = fopen("/spiffs/settings.json", "r");
    if (f == NULL) {
        std::cout << "json: Failed to open file" << "\n";
        return;
    }

    std::cout << "json: Reading json file" << "\n";
    std::ifstream in("/spiffs/settings.json");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());

    std::cout << "json: Getting data" << "\n";
    cJSON *root = cJSON_Parse(contents.c_str());
    cJSON *setting = NULL;
    cJSON *settings = NULL;

    std::cout << "json: Parse settings information" << "\n";
    settings = cJSON_GetObjectItemCaseSensitive(root, "settings");
   
    settingsVariables temp;
    variables.clear();
    std::cout << "json: Extract info" << "\n";
    cJSON_ArrayForEach(setting, settings) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(setting, "name");
        cJSON *value = cJSON_GetObjectItemCaseSensitive(setting, "value");
        temp.name = name->valuestring;
        if (variables_fields.find(temp.name) == variables_fields.end()) {
            variables_fields.insert({temp.name, variables.size()});
        }
        if (!cJSON_IsNumber(value)) {
            temp.textValue = value->valuestring;
            temp.type = "text";
            temp.numberValue = 0;
            variables.push_back(temp);
        } else {
            temp.textValue.empty();
            temp.numberValue = value->valuedouble;
            temp.type = "number";
            variables.push_back(temp);
        }
    }

    // Print acquired data
    std::cout << "\nModule-specific settings:\n\n";
    for (auto it : variables) {
        std::cout << it.name << ": ";
        if (it.type == "text") {
            std::cout << it.textValue << "\n";
        } else if (it.type == "number") {
            std::cout << it.numberValue << "\n";
        }
    }
    std::cout << "\n";
    
    cJSON_Delete(root);

    fclose(f);
    Puara::unmount_spiffs();
}


void Puara::write_config_json() {
    
    std::cout << "SPIFFS: Mounting FS" << "\n";
    Puara::mount_spiffs();

    std::cout << "SPIFFS: Opening config.json file" << "\n";
    FILE* f = fopen("/spiffs/config.json", "w");
    if (f == NULL) {
        std::cout << "SPIFFS: Failed to open config.json file" << "\n";
        return;
    }

    cJSON *device_json = NULL;
    cJSON *id_json = NULL;
    cJSON *author_json = NULL;
    cJSON *institution_json = NULL;
    cJSON *APpasswd_json = NULL;
    cJSON *wifiSSID_json = NULL;
    cJSON *wifiPSK_json = NULL;
    cJSON *persistentAP_json = NULL;
    cJSON *oscIP1_json = NULL;
    cJSON *oscPORT1_json = NULL;
    cJSON *oscIP2_json = NULL;
    cJSON *oscPORT2_json = NULL;
    cJSON *localPORT_json = NULL;

    cJSON *root = cJSON_CreateObject();

    device_json = cJSON_CreateString(device.c_str());
    cJSON_AddItemToObject(root, "device", device_json);
    
    id_json = cJSON_CreateNumber(id);
    cJSON_AddItemToObject(root, "id", id_json);
    
    author_json = cJSON_CreateString(author.c_str());
    cJSON_AddItemToObject(root, "author", author_json);
    
    institution_json = cJSON_CreateString(institution.c_str());
    cJSON_AddItemToObject(root, "institution", institution_json);
    
    APpasswd_json = cJSON_CreateString(APpasswd.c_str());
    cJSON_AddItemToObject(root, "APpasswd", APpasswd_json);
    
    wifiSSID_json = cJSON_CreateString(wifiSSID.c_str());
    cJSON_AddItemToObject(root, "wifiSSID", wifiSSID_json);
    
    wifiPSK_json = cJSON_CreateString(wifiPSK.c_str());
    cJSON_AddItemToObject(root, "wifiPSK", wifiPSK_json);

    persistentAP_json = cJSON_CreateNumber(persistentAP);
    cJSON_AddItemToObject(root, "persistentAP", persistentAP_json);
    
    oscIP1_json = cJSON_CreateString(oscIP1.c_str());
    cJSON_AddItemToObject(root, "oscIP1", oscIP1_json);
    
    oscPORT1_json = cJSON_CreateNumber(oscPORT1);
    cJSON_AddItemToObject(root, "oscPORT1", oscPORT1_json);
    
    oscIP2_json = cJSON_CreateString(oscIP2.c_str());
    cJSON_AddItemToObject(root, "oscIP2", oscIP2_json);
    
    oscPORT2_json = cJSON_CreateNumber(oscPORT2);
    cJSON_AddItemToObject(root, "oscPORT2", oscPORT2_json);
    
    localPORT_json = cJSON_CreateNumber(localPORT);
    cJSON_AddItemToObject(root, "localPORT", localPORT_json);

    std::cout << "json: Data stored:" << "\n";
    std::cout << "\ndevice: " << device << "\n";
    std::cout << "id: " << id << "\n";
    std::cout << "author: " << author << "\n";
    std::cout << "institution: " << institution << "\n";
    std::cout << "APpasswd: " << APpasswd << "\n";
    std::cout << "wifiSSID: " << wifiSSID << "\n";
    std::cout << "wifiPSK: " << wifiPSK << "\n";
    std::cout << "persistentAP: " << persistentAP << "\n";
    std::cout << "oscIP1: " << oscIP1 << "\n";
    std::cout << "oscPORT1: " << oscPORT1 << "\n";
    std::cout << "oscIP2: " << oscIP2 << "\n";
    std::cout << "oscPORT2: " << oscPORT2 << "\n";
    std::cout << "localPORT: " << localPORT << "\n\n";

    // Save to config.json
    std::cout << "write_config_json: Serializing json" << "\n";
    std::string contents = cJSON_Print(root);
    std::cout << "SPIFFS: Saving file" << "\n";
    fprintf(f, "%s", contents.c_str());
    std::cout << "SPIFFS: closing" << "\n";
    fclose(f);

    std::cout << "write_config_json: Delete json entity" << "\n";
    cJSON_Delete(root);

    std::cout << "SPIFFS: umounting FS" << "\n";
    Puara::unmount_spiffs();
}

void Puara::write_settings_json() {
    
    std::cout << "SPIFFS: Mounting FS" << "\n";
    Puara::mount_spiffs();

    std::cout << "SPIFFS: Opening settings.json file" << "\n";
    FILE* f = fopen("/spiffs/settings.json", "w");
    if (f == NULL) {
        std::cout << "SPIFFS: Failed to open settings.json file" << "\n";
        return;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *settings = cJSON_CreateArray();
    cJSON *setting = NULL;
    cJSON *data = NULL;
    cJSON_AddItemToObject(root, "settings", settings);

    for (auto it : variables) {
        setting = cJSON_CreateObject();
        cJSON_AddItemToArray(settings, setting);
        data = cJSON_CreateString(it.name.c_str());
        cJSON_AddItemToObject(setting, "name", data);
        if (it.type == "text") {
            data = cJSON_CreateString(it.textValue.c_str());
        } else if (it.type == "number") {
            data = cJSON_CreateNumber(it.numberValue);
        }
        cJSON_AddItemToObject(setting, "value", data);
    }

    // Save to settings.json
    std::cout << "write_settings_json: Serializing json" << "\n";
    std::string contents = cJSON_Print(root);
    std::cout << "SPIFFS: Saving file" << "\n";
    fprintf(f, "%s", contents.c_str());
    std::cout << "SPIFFS: closing" << "\n";
    fclose(f);

    std::cout << "write_settings_json: Delete json entity" << "\n";
    cJSON_Delete(root);

    std::cout << "SPIFFS: umounting FS" << "\n";
    Puara::unmount_spiffs();
}

std::string Puara::get_dmi_name() {
    return dmiName;
}

std::string Puara::prepare_index() {
    Puara::mount_spiffs();
    std::cout << "http (spiffs): Reading index file" << "\n";
    std::ifstream in("/spiffs/index.html");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    // Put the module info on the HTML before send response
    Puara::find_and_replace("%DMINAME%", Puara::dmiName, contents);
    if (Puara::StaIsConnected) {
        Puara::find_and_replace("%STATUS%", "Currently connected on "
                                             "<strong style=\"color:Tomato;\">" + 
                                             Puara::wifiSSID + "</strong> network", 
                                             contents);
    } else {
        Puara::find_and_replace("%STATUS%", "Currently not connected to any network", 
                                 contents);
    }
    Puara::find_and_replace("%CURRENTSSID%", Puara::currentSSID, contents);
    Puara::find_and_replace("%CURRENTPSK%", Puara::wifiPSK, contents);
    Puara::checkmark("%CURRENTPERSISTENT%", Puara::persistentAP, contents);
    Puara::find_and_replace("%DEVICENAME%", Puara::device, contents);
    Puara::find_and_replace("%CURRENTOSC1%", Puara::oscIP1, contents);
    Puara::find_and_replace("%CURRENTPORT1%", Puara::oscPORT1, contents);
    Puara::find_and_replace("%CURRENTOSC2%", Puara::oscIP2, contents);
    Puara::find_and_replace("%CURRENTPORT2%", Puara::oscPORT1, contents);
    Puara::find_and_replace("%CURRENTLOCALPORT%", Puara::localPORT, contents);
    Puara::find_and_replace("%CURRENTSSID2%", Puara::wifiSSID, contents);
    Puara::find_and_replace("%CURRENTIP%", Puara::currentSTA_IP, contents);
    Puara::find_and_replace("%CURRENTAPIP%", Puara::currentAP_IP, contents);
    Puara::find_and_replace("%CURRENTSTAMAC%", Puara::currentSTA_MAC, contents);
    Puara::find_and_replace("%CURRENTAPMAC%", Puara::currentAP_MAC, contents);
    std::ostringstream tempBuf;
    tempBuf << std::setfill('0') << std::setw(3) << std::hex << Puara::id;
    Puara::find_and_replace("%MODULEID%", tempBuf.str(), contents);
    Puara::find_and_replace("%MODULEAUTH%", Puara::author, contents);
    Puara::find_and_replace("%MODULEINST%", Puara::institution, contents);
    Puara::find_and_replace("%MODULEVER%", Puara::VERSION, contents);

    Puara::unmount_spiffs();

    return contents;
}

esp_err_t Puara::index_get_handler(httpd_req_t *req) {

    std::string prepared_index = prepare_index();
    httpd_resp_sendstr(req, prepared_index.c_str());

    return ESP_OK;
}

esp_err_t Puara::settings_get_handler(httpd_req_t *req) {

    Puara::mount_spiffs();
    std::cout << "http (spiffs): Reading settings file" << "\n";
    std::ifstream in("/spiffs/settings.html");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());

    std::cout << "settings_get_handler: Adding variables to HTML" << "\n";
    std::string settings;
    for (auto it : variables) {
        if (it.type == "text") {
            settings.append("<div class=\"row\"><div class=\"col-25\"><label for=\"%PARAMETER%\">%PARAMETER%</label></div><div class=\"col-75\"><input type=\"text\" id=\"%PARAMETER%\" name=\"%PARAMETER%\" value=\"%PARAMETERVALUE%\"></div></div>");
            find_and_replace("%PARAMETERVALUE%", it.textValue, settings);
            find_and_replace("%PARAMETER%", it.name, settings);
        } else if (it.type == "number") {
            settings.append("<div class=\"row\"><div class=\"col-25\"><label for=\"%PARAMETER%\">%PARAMETER%</label></div><div class=\"col-75\"><input type=\"number\" step=\"0.000001\" id=\"%PARAMETER%\" name=\"%PARAMETER%\" value=\"%PARAMETERVALUE%\"></div></div>");
            find_and_replace("%PARAMETERVALUE%", it.numberValue, settings);
            find_and_replace("%PARAMETER%", it.name, settings);
        }
    }
    find_and_replace("%DATAFROMMODULE%", settings, contents);
    httpd_resp_sendstr(req, contents.c_str());
    
    return ESP_OK;
}

esp_err_t Puara::settings_post_handler(httpd_req_t *req) {
    char buf[200];
    
    int api_return, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((api_return = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (api_return == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        std::string str_buf = convertToString(buf);
        std::string str_token;
        std::string field;
        size_t pos = 0;
        size_t field_pos = 0;
        std::string delimiter = "&";
        std::string field_delimiter = "=";
        // adding delimiter to process last variable in the loop
        str_buf.append(delimiter);

        std::cout << "Settings stored:" << "\n";
        while ((pos = str_buf.find(delimiter)) != std::string::npos) {
            str_token = str_buf.substr(0, pos);
            field_pos = str_buf.find(field_delimiter);
            field = str_token.substr(0, field_pos);
            str_token.erase(0, field_pos + field_delimiter.length());
            std::cout << field << ": ";
            if (variables.at(variables_fields.at(field)).type == "text") {
                variables.at(variables_fields.at(field)).textValue = urlDecode(str_token);
            } else if (variables.at(variables_fields.at(field)).type == "number") {
                variables.at(variables_fields.at(field)).numberValue = std::stod(str_token);
            }
            std::cout << str_token << "\n";
            str_buf.erase(0, pos + delimiter.length());
        }
        std::cout << "\n";
        remaining -= api_return;
    }

    write_settings_json();
    mount_spiffs();
    std::cout << "http (spiffs): Reading saved.html file" << "\n";
    std::ifstream in("/spiffs/saved.html");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    httpd_resp_sendstr(req, contents.c_str());
    unmount_spiffs();

    return ESP_OK;
}

esp_err_t Puara::get_handler(httpd_req_t *req) {

    const char* resp_str = (const char*) req->user_ctx;
    Puara::mount_spiffs();
    std::cout << "http (spiffs): Reading requested file" << "\n";
    std::ifstream in(resp_str);
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    httpd_resp_sendstr(req, contents.c_str());
    
    Puara::unmount_spiffs();

    return ESP_OK;
}

esp_err_t Puara::style_get_handler(httpd_req_t *req) {

    const char* resp_str = (const char*) req->user_ctx;
    Puara::mount_spiffs();
    std::cout << "http (spiffs): Reading style.css file" << "\n";
    std::ifstream in(resp_str);
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    httpd_resp_set_type(req, "text/css");
    httpd_resp_sendstr(req, contents.c_str());
    
    Puara::unmount_spiffs();

    return ESP_OK;
}

esp_err_t Puara::scan_get_handler(httpd_req_t *req) {

    const char* resp_str = (const char*) req->user_ctx;
    Puara::mount_spiffs();
    std::cout << "http (spiffs): Reading scan.html file" << "\n";
    std::ifstream in(resp_str);
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    wifi_scan();
    find_and_replace("%SSIDS%", wifiAvailableSsid, contents);
    httpd_resp_sendstr(req, contents.c_str());
    
    Puara::unmount_spiffs();

    return ESP_OK;
}

// esp_err_t Puara::update_get_handler(httpd_req_t *req) {

//     const char* resp_str = (const char*) req->user_ctx;
//     Puara::mount_spiffs();
//     std::cout << "http (spiffs): Reading update.html file" << "\n";
//     std::ifstream in(resp_str);
//     std::string contents((std::istreambuf_iterator<char>(in)), 
//     std::istreambuf_iterator<char>());
//     //httpd_resp_set_type(req, "text/html");
//     httpd_resp_sendstr(req, contents.c_str());
    
//     Puara::unmount_spiffs();

//     return ESP_OK;
// }

esp_err_t Puara::index_post_handler(httpd_req_t *req) {
    char buf[200];
    bool ret_flag = false;
    
    int api_return, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((api_return = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (api_return == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        std::string str_buf = convertToString(buf);
        std::string str_token;
        std::string field;
        size_t pos = 0;
        size_t field_pos = 0;
        std::string delimiter = "&";
        std::string field_delimiter = "=";
        // adding delimiter to process last variable in the loop
        str_buf.append(delimiter);
        bool checkbox_persistentAP = false;

        while ((pos = str_buf.find(delimiter)) != std::string::npos) {
            str_token = str_buf.substr(0, pos);
            field_pos = str_buf.find(field_delimiter);
            field = str_token.substr(0, field_pos);
            str_token.erase(0, field_pos + field_delimiter.length());
            if (config_fields.find(field) != config_fields.end()) {
                switch (config_fields.at(field)) {
                    case 1:
                        std::cout << "SSID: " << str_token << "\n";
                        if ( !str_token.empty() ) { 
                            wifiSSID = urlDecode(str_token);
                        } else {
                            std::cout << "SSID empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 2:
                        std::cout << "APpasswd: " << str_token << "\n";
                        if ( !str_token.empty() ) { 
                            APpasswdVal1 = urlDecode(str_token); 
                        } else {
                            std::cout << "APpasswd empty! Keeping the stored value" << "\n";
                            APpasswdVal1.clear();
                        };
                        break;
                    case 3:
                        std::cout << "APpasswdValidate: " << str_token << "\n";
                        if ( !str_token.empty() ) { 
                            APpasswdVal2 = urlDecode(str_token);
                        } else {
                            std::cout << "APpasswdValidate empty! Keeping the stored value" << "\n";
                            APpasswdVal2.clear();
                        };
                        break;
                    case 4:
                        std::cout << "oscIP1: " << str_token << "\n";
                        if ( !str_token.empty() ) {
                            oscIP1 = str_token;
                        } else {
                            std::cout << "oscIP1 empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 5:
                        std::cout << "oscPORT1: " << str_token << "\n";
                        if ( !str_token.empty() ) {
                            oscPORT1 = stoi(str_token);
                        } else {
                            std::cout << "oscPORT1 empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 6:
                        std::cout << "oscIP2: " << str_token << "\n";
                        if ( !str_token.empty() ) {
                            oscIP2 = str_token;
                        } else {
                            std::cout << "oscIP2 empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 7:
                        std::cout << "oscPORT2: " << str_token << "\n";
                        if ( !str_token.empty() ) {
                            oscPORT2 = stoi(str_token);
                        } else {
                            std::cout << "oscPORT2 empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 8:
                        std::cout << "password: " << str_token << "\n";
                        if ( !str_token.empty() ) { 
                            wifiPSK = urlDecode(str_token);
                        } else {
                            std::cout << "password empty! Keeping the stored value" << "\n";
                        }
                        break;
                    case 9:
                        std::cout << "Rebooting\n";
                        ret_flag = true;
                        break;
                    case 10:
                        std::cout << "persistentAP: " << str_token << "\n";
                        checkbox_persistentAP = true;
                        break;
                    case 11:
                        std::cout << "localPORT: " << str_token << "\n";
                        if ( !str_token.empty() ) {
                            localPORT = stoi(str_token);
                        } else {
                            std::cout << "localPORT empty! Keeping the stored value" << "\n";
                        }
                        break;
                    default:
                        std::cout << "Error, no match for config field to store received data\n";
                        break; 
                }
            } else {
                std::cout << "Error, no match for config field to store received data: " << field << "\n";
            }
            str_buf.erase(0, pos + delimiter.length());
        }

        // processing some post info
        if ( APpasswdVal1 == APpasswdVal2 && !APpasswdVal1.empty() && APpasswdVal1.length() > 7 ) {
            APpasswd = APpasswdVal1;
            std::cout << "Puara password changed!\n";
        } else {
            std::cout << "Puara password doesn't match or shorter than 8 characteres. Passwork not changed.\n";
        }
        persistentAP = checkbox_persistentAP;
        APpasswdVal1.clear(); APpasswdVal2.clear();

        remaining -= api_return;
    }

    if (ret_flag) {
        mount_spiffs();
        std::cout << "http (spiffs): Reading reboot.html file" << "\n";
        std::ifstream in("/spiffs/reboot.html");
        std::string contents((std::istreambuf_iterator<char>(in)), 
        std::istreambuf_iterator<char>());
        httpd_resp_sendstr(req, contents.c_str());
        unmount_spiffs();
        std::cout <<  "\nRebooting...\n";
        xTaskCreate(&Puara::reboot_with_delay, "reboot_with_delay", 1024, NULL, 10, NULL);
    } else {
        write_config_json();
        mount_spiffs();
        std::cout << "http (spiffs): Reading saved.html file" << "\n";
        std::ifstream in("/spiffs/saved.html");
        std::string contents((std::istreambuf_iterator<char>(in)), 
        std::istreambuf_iterator<char>());
        httpd_resp_sendstr(req, contents.c_str());
        unmount_spiffs();
    }

    return ESP_OK;
}

void Puara::find_and_replace(std::string old_text, std::string new_text, std::string & str) {

    std::size_t old_text_position = str.find(old_text);
    while (old_text_position!=std::string::npos) {
        str.replace(old_text_position,old_text.length(),new_text);
        old_text_position = str.find(old_text);
    }
    std::cout << "http (find_and_replace): Success" << "\n";
}

void Puara::find_and_replace(std::string old_text, double new_number, std::string & str) {

    std::size_t old_text_position = str.find(old_text);
    while (old_text_position!=std::string::npos) {
        std::string conversion = std::to_string(new_number);
        str.replace(old_text_position,old_text.length(),conversion);
        old_text_position = str.find(old_text);
    }
    std::cout << "http (find_and_replace): Success" << "\n";
}

void Puara::find_and_replace(std::string old_text, unsigned int new_number, std::string & str) {

    std::size_t old_text_position = str.find(old_text);
    while (old_text_position!=std::string::npos) {
        std::string conversion = std::to_string(new_number);
        str.replace(old_text_position,old_text.length(),conversion);
        old_text_position = str.find(old_text);
    }
    std::cout << "http (find_and_replace): Success" << "\n";
}

void Puara::checkmark(std::string old_text, bool value, std::string & str) {

    std::size_t old_text_position = str.find(old_text);
    if (old_text_position!=std::string::npos) {
        std::string conversion;
        if (value) {
            conversion = "checked";
        } else {
            conversion = "";
        }
        str.replace(old_text_position,old_text.length(),conversion);
        std::cout << "http (checkmark): Success" << "\n";
    } else {
        std::cout << "http (checkmark): Could not find the requested string" << "\n";
    }
}

httpd_handle_t Puara::start_webserver(void) {
    
    if (!ApStarted) {
        std::cout << "start_webserver: Cannot start webserver: AP and STA not initializated" << "\n";
        return NULL;
    }
    Puara::webserver = NULL;

    Puara::webserver_config.task_priority      = tskIDLE_PRIORITY+5;
    Puara::webserver_config.stack_size         = 4096;
    Puara::webserver_config.core_id            = tskNO_AFFINITY;
    Puara::webserver_config.server_port        = 80;
    Puara::webserver_config.ctrl_port          = 32768;
    Puara::webserver_config.max_open_sockets   = 7;
    Puara::webserver_config.max_uri_handlers   = 9;
    Puara::webserver_config.max_resp_headers   = 9;
    Puara::webserver_config.backlog_conn       = 5;
    Puara::webserver_config.lru_purge_enable   = true;
    Puara::webserver_config.recv_wait_timeout  = 5;
    Puara::webserver_config.send_wait_timeout  = 5;
    Puara::webserver_config.global_user_ctx = NULL;
    Puara::webserver_config.global_user_ctx_free_fn = NULL;
    Puara::webserver_config.global_transport_ctx = NULL;
    Puara::webserver_config.global_transport_ctx_free_fn = NULL;
    Puara::webserver_config.open_fn = NULL;
    Puara::webserver_config.close_fn = NULL;
    Puara::webserver_config.uri_match_fn = NULL;

    Puara::index.uri = "/";
    Puara::index.method    = HTTP_GET,
    Puara::index.handler   = index_get_handler,
    Puara::index.user_ctx  = (char*)"/spiffs/index.html";

    Puara::indexpost.uri = "/";
    Puara::indexpost.method    = HTTP_POST,
    Puara::indexpost.handler   = index_post_handler,
    Puara::indexpost.user_ctx  = (char*)"/spiffs/index.html";

    Puara::style.uri = "/style.css";
    Puara::style.method    = HTTP_GET,
    Puara::style.handler   = style_get_handler,
    Puara::style.user_ctx  = (char*)"/spiffs/style.css";

    Puara::factory.uri = "/factory.html";
    Puara::factory.method    = HTTP_GET,
    Puara::factory.handler   = get_handler,
    Puara::factory.user_ctx  = (char*)"/spiffs/factory.html";

    Puara::reboot.uri = "/reboot.html";
    Puara::reboot.method    = HTTP_GET,
    Puara::reboot.handler   = get_handler,
    Puara::reboot.user_ctx  = (char*)"/spiffs/reboot.html";

    Puara::scan.uri = "/scan.html";
    Puara::scan.method    = HTTP_GET,
    Puara::scan.handler   = scan_get_handler,
    Puara::scan.user_ctx  = (char*)"/spiffs/scan.html";

    // Puara::update.uri = "/update.html";
    // Puara::update.method    = HTTP_GET,
    // Puara::update.handler   = get_handler,
    // Puara::update.user_ctx  = (char*)"/spiffs/update.html";

    Puara::settings.uri = "/settings.html";
    Puara::settings.method    = HTTP_GET,
    Puara::settings.handler   = settings_get_handler,
    Puara::settings.user_ctx  = (char*)"/spiffs/settings.html";

    Puara::settingspost.uri = "/settings.html";
    Puara::settingspost.method    = HTTP_POST,
    Puara::settingspost.handler   = settings_post_handler,
    Puara::settingspost.user_ctx  = (char*)"/spiffs/settings.html";

    // Start the httpd server
    std::cout << "webserver: Starting server on port: " << webserver_config.server_port << "\n";
    if (httpd_start(&webserver, &webserver_config) == ESP_OK) {
        // Set URI handlers
        std::cout << "webserver: Registering URI handlers" << "\n";
        httpd_register_uri_handler(webserver, &index);
        httpd_register_uri_handler(webserver, &indexpost);
        httpd_register_uri_handler(webserver, &style);
        httpd_register_uri_handler(webserver, &scan);
        httpd_register_uri_handler(webserver, &factory);
        httpd_register_uri_handler(webserver, &reboot);
        // httpd_register_uri_handler(webserver, &update);
        httpd_register_uri_handler(webserver, &settings);
        httpd_register_uri_handler(webserver, &settingspost);
        return webserver;
    }

    std::cout << "webserver: Error starting server!" << "\n";
    return NULL;
}

void Puara::stop_webserver(void) {
    // Stop the httpd server
    httpd_stop(webserver);
}

std::string Puara::convertToString(char* a) {
    std::string s(a);
    return s;
}

void Puara::interpret_serial(void *pvParameter) {
    while (1) {
        if ( !serial_data_str.empty() ) {
            if ( serial_data_str.find("reset") != std::string::npos ) { //data_str == "reset") {
                std::cout <<  "\nRebooting...\n";
                xTaskCreate(&Puara::reboot_with_delay, "reboot_with_delay", 1024, NULL, 10, NULL);
            } else if ( serial_data_str.find("reboot") != std::string::npos ) {
                std::cout <<  "\nRebooting...\n";
                xTaskCreate(&Puara::reboot_with_delay, "reboot_with_delay", 1024, NULL, 10, NULL);
            } else {
                std::cout << "\nI don´t recognize the command: \n" << serial_data_str << std::endl;
            }
            serial_data_str.clear();
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void Puara::serial_monitor(void *pvParameters) {
    const int uart_num0 = 0; //UART port 0
    uart_config_t uart_config0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,    //UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };

    //Configure UART1 parameters
    uart_param_config(uart_num0, &uart_config0);

    uart_set_pin(uart_num0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    //Install UART driver (we don't need an event queue here)
    //In this example we don't even use a buffer for sending data.
    uart_driver_install(uart_num0, UART_FIFO_LEN + 1, 0, 0, NULL, 0);

    while(1) {
        //Read data from UART
        serial_data_length = uart_read_bytes(uart_num0, serial_data, UART_FIFO_LEN, 500 / portTICK_RATE_MS);
        if (serial_data_length > 0) {
            serial_data_str = convertToString(serial_data);
            memset(serial_data, 0, sizeof serial_data);
            uart_flush(uart_num0);
        }
    }
}

bool Puara::start_serial_listening() {
    std::cout << "starting serial monitor \n";
    xTaskCreate(serial_monitor, "serial_monitor", 1024, NULL, 10, NULL);
    xTaskCreate(interpret_serial, "interpret_serial", 2048, NULL, 10, NULL);
    return 1;
}

void Puara::reboot_with_delay(void *pvParameter) {
    vTaskDelay(reboot_delay / portTICK_RATE_MS);
    esp_restart();
}

void Puara::start_mdns_service(const char * device_name, const char * instance_name) {
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        std::cout << "MDNS Init failed: " << err << "\n";
        return;
    }
    //set hostname
    ESP_ERROR_CHECK(mdns_hostname_set(device_name));
    //set default instance
    ESP_ERROR_CHECK(mdns_instance_name_set(instance_name));
    std::cout << "MDNS Init completed. Device name: " << device_name << "\n\n";
}

void Puara::start_mdns_service(std::string device_name, std::string instance_name) {
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        std::cout << "MDNS Init failed: " << err << "\n";
        return;
    }
    //set hostname
    ESP_ERROR_CHECK(mdns_hostname_set(device_name.c_str()));
    //set default instance
    ESP_ERROR_CHECK(mdns_instance_name_set(instance_name.c_str()));
    std::cout << "MDNS Init completed. Device name: " << device_name << "\n\n";
}

void Puara::wifi_scan(void) {

    uint16_t number = wifiScanSize;
    wifi_ap_record_t ap_info[wifiScanSize];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    std::cout << "wifi_scan: Total APs scanned = " << ap_count << "\n";
    wifiAvailableSsid.clear();
    for (int i = 0; (i < wifiScanSize) && (i < ap_count); i++) {
        wifiAvailableSsid.append("<strong>SSID: </strong>");
        wifiAvailableSsid.append(reinterpret_cast<const char*>(ap_info[i].ssid));
        wifiAvailableSsid.append("<br>      (RSSI: ");
        wifiAvailableSsid.append(std::to_string(ap_info[i].rssi));
        wifiAvailableSsid.append(", Channel: ");
        wifiAvailableSsid.append(std::to_string(ap_info[i].primary));
        wifiAvailableSsid.append(")<br>");
    }
}

std::string Puara::urlDecode(std::string text) {
      
    std::string escaped;
    for (auto i = text.begin(), nd = text.end(); i < nd; ++i){
      auto c = ( *i );
       switch(c) {
        case '%':
          if (i[1] && i[2]) {
              char hs[]{ i[1], i[2] };
              escaped += static_cast<char>(strtol(hs, nullptr, 16));
              i += 2;
          }
          break;
        case '+':
          escaped += ' ';
          break;
        default:
          escaped += c;
      }
    }
    return escaped;
}

double Puara::getVarNumber(std::string varName) {
    return variables.at(variables_fields.at(varName)).numberValue;
}
        
std::string Puara::getVarText(std::string varName) {
    return variables.at(variables_fields.at(varName)).textValue;
}

void Puara::start_communication(void) {
    // this needs to run after dmiName is defined with read_config_json()
    std::cout << "Start communication (libmapper/OSC)"  << "\n";
    lmDev = new mapper::Device(dmiName);
    // std::cout << "Start task to update messages"  << "\n";
    // xTaskCreate(&Puara::update_Messages, "update_Messages", 2048, NULL, 5, NULL);
}

void Puara::createMessage(std::string name, int length, std::string unit, float min, float max, std::string mode) {
    if (lm_fields.find(name) == lm_fields.end()) {
        lm_fields.insert({name, lmSignals.size()});
    }
    lmMin.push_back(min);
    lmMax.push_back(max);
    lmSignals.push_back(lmDev->add_signal(mapper::Direction::OUTGOING, name, length,
                                            mapper::Type::FLOAT, unit, &lmMin.at(lm_fields.at(name)), &lmMax.at(lm_fields.at(name))));
    
    //lmSignals.at(lm_fields.at(name)).set_property("mode", mode);
}

void Puara::setMessage(std::string name, float value) {
    if (StaIsConnected || ApStarted) {
        if (lm_fields.find(name) != lm_fields.end()) {
            lmSignals.at(lm_fields.at(name)).set_value(value);
            std::string osc_namespace = "/" + dmiName + "/" + name;
            if (oscIP1 != "0.0.0.0" || oscIP1 != "") {
                lo_send(lo_oscIP1, osc_namespace.c_str(), "f", value);
            }
            if (oscIP2 != "0.0.0.0" || oscIP2 != "") {
                lo_send(lo_oscIP2, osc_namespace.c_str(), "f", value);
            }
        }
    }
}
