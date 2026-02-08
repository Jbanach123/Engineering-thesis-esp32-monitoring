#include "wifi.h"
#include "esp_log.h"
#include "config.h"
#include "lwip/ip4_addr.h"
#include "esp_event.h"
#include "esp_wifi.h"

static const char *TAG = "wifi";

// Event handler for Wi-Fi events
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {

    // Attempt to connect on start and reconnect on disconnect
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Rozłączono z Wi-Fi, ponawiam...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Uzyskano IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Static IP configuration
void wifi_init_sta(void) {

    // TCP/IP and event loop initialization
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 137, 102);
    IP4_ADDR(&ip_info.gw, 192, 168, 137, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    // Stop DHCP client and set static IP
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // Set Wi-Fi mode and configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi STA uruchomione");
}