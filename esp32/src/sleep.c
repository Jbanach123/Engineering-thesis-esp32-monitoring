#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sleep.h"
#include "config.h"
#include "webserver.h"
#include "led.h"

static const char *TAG = "sleep";

// Inform about system activity
void sleep_notify_activity(void)
{
    // Update last activity timestamp
    last_system_activity = esp_timer_get_time() / 1000;

    if (in_modem_sleep) {
        sleep_exit_modem_sleep();
    }
}


void sleep_exit_modem_sleep(void)
{ 
    // Disable  Wi-Fi power save mode
    esp_wifi_set_ps(WIFI_PS_NONE);
    led_on();
    in_modem_sleep = false;

    ESP_LOGI(TAG, "Wyjście z MODEM SLEEP");
}


// Sleep management task
static void sleep_task(void *arg)
{
    last_system_activity = esp_timer_get_time() / 1000;

    while (1) {
        int64_t now = esp_timer_get_time() / 1000;

        // Enter modem sleep after inactivity timeout
        if (!client_active &&
            !in_modem_sleep &&
            (now - last_system_activity >= IDLE_TIMEOUT_MS)) {

            ESP_LOGI(TAG, "Idle timeout - WiFi w modem sleep");
            
            //Enable Wi-Fi power save mode
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
            led_off();
            in_modem_sleep = true;
        }

        // Inactivity check interval
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Initialize sleep management
void sleep_manager_init(void)
{
    // Create sleep management task pinned to core 0
    xTaskCreatePinnedToCore(
        sleep_task,
        "sleep_task",
        3072,
        NULL,
        4,
        NULL,
        0   // core 0 – logika systemowa
    );

    ESP_LOGI(TAG, "Sleep manager uruchomiony");
}
