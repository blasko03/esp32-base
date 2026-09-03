#include "esp_http_server.h"
#include <esp_log.h>

static const char *TAG = "HTTP_SERVER";

static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *response = "Hello from ESP32-C5!";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t root_uri = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = root_get_handler,
    .user_ctx = NULL
};

httpd_handle_t start_api_http_server() {
    httpd_handle_t server = NULL;
    
    const httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting HTTP server on port: '%d'", config.server_port);
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root_uri);
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server!");
    return NULL;
}