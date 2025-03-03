#pragma once
#include "puara_spiffs.hpp"
#include <string.h> //Requires by memset
#include <esp_http_server.h>
#include <mdns.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

// Hi there ! You shouldn't need to change anything in this file, just call setup_oscquery_server

inline PuaraAPI::SPIFFS spiffs;

inline httpd_handle_t oscqserver = nullptr;

inline std::string oscq_namespace;
inline std::string oscq_host_info;

inline char query_str_buf[1024] = {0};
inline char query_str_value[1024] = {0};

void read_oscq_metadata_from_settings() {
  spiffs.config_spiffs();
  spiffs.mount_spiffs();

  FILE* f = fopen("/spiffs/settings.json", "r");
  if(f == NULL)
  {
    std::cout << "oscquery namespace: Failed to open settings.json" << std::endl;
    return;
  }

  std::cout << "json: Reading json file" << std::endl;
  std::ifstream in("/spiffs/settings.json");
  nlohmann::json data = nlohmann::json::parse(in);
  oscq_namespace = data["oscquery_namespace"].dump();
  oscq_host_info = data["oscquery_host_info"].dump();
  fclose(f);
  spiffs.unmount_spiffs();
}

esp_err_t get_req_handler(httpd_req_t *req)
{
  httpd_req_get_url_query_str(req, query_str_buf, 1024);
  httpd_query_key_value(query_str_buf, "HOST_INFO", query_str_value, 1024);
  if(std::string(query_str_buf).find("HOST_INFO") != std::string::npos) {
    memset(query_str_buf, '\0', 1024);
    return httpd_resp_send(req, oscq_host_info.c_str(), HTTPD_RESP_USE_STRLEN);
  }
  memset(query_str_buf, '\0', 1024);
  return httpd_resp_send(req, oscq_namespace.c_str(), HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t uri_get = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = get_req_handler,
  .user_ctx = NULL};

void setup_oscquery_server()
{
  ESP_ERROR_CHECK(mdns_service_add(nullptr, "_oscjson", "_tcp", puara.getLocalPORT(), nullptr, 0));
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = puara.getLocalPORT();
  // puara module uses port 32768 so we use the next one.
  config.ctrl_port = 32769;
  // populate the oscquery metadata from the settings.json file.
  read_oscq_metadata_from_settings();
  if (httpd_start(&oscqserver, &config) == ESP_OK)
  {
    httpd_register_uri_handler(oscqserver, &uri_get);
  }
  Udp.begin(puara.getLocalPORT());
}
