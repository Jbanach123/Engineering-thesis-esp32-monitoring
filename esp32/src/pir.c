#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "pir.h"
#include "snapshot.h"
#include "config.h"
#include "led.h"
#include "sleep.h"

// PIR sigal debounce and motion cooldown times
#define PIR_DEBOUNCE_MS      200
#define MOTION_COOLDOWN_MS 60000

static const char *TAG = "pir";

// Queue used to transfer PIR events from ISR to task context
static QueueHandle_t pir_evt_queue = NULL;

// Last PIR event timestamp
static volatile int64_t last_pir_time = 0;

// ISR dla PIR
static void IRAM_ATTR pir_isr_handler(void* arg)
{
    int64_t now = esp_timer_get_time() / 1000;

    if (now - last_pir_time > PIR_DEBOUNCE_MS) {
        last_pir_time = now;
        uint32_t evt = 1;
        BaseType_t hpw = pdFALSE;

        // Send motion event to PIR task
        xQueueSendFromISR(pir_evt_queue, &evt, &hpw);

        // Perform context switch if required
        if (hpw) portYIELD_FROM_ISR();
    }
}

// PIR task processing motion events
static void pir_task(void *arg)
{
    uint32_t evt;
    while (1) {

        // Wait for motion event from ISR
        if (xQueueReceive(pir_evt_queue, &evt, portMAX_DELAY)) {

            int64_t now = esp_timer_get_time() / 1000;

            // If motion not already active, and cooldown elapsed
            if (!motion_active) {

                // Check cooldown period before triggering new motion event
                if (now - last_event_time > MOTION_COOLDOWN_MS) {
                    motion_active = true;
                    last_motion_time = now;
                    last_event_time = now;
                    sleep_notify_activity();

                    led_off();
                    esp_wifi_set_ps(WIFI_PS_NONE);

                    ESP_LOGI(TAG, "Ruch rozpoczęty - wysyłam snapshot");
                    send_motion_snapshot();

                    led_on();
                } else {
                    ESP_LOGI(TAG, "Ruch w cooldownie - pomijam");
                }
            } else {
                // Motion is still ongoing
                last_motion_time = now;
                ESP_LOGI(TAG, "Ruch kontynuowany");
            }
        }
    }
}

// Initialize PIR motion detector
void pir_init(void)
{
    // Create PIR event queue
    pir_evt_queue = xQueueCreate(8, sizeof(uint32_t));

    // Configure PIR GPIO input with rising edge interrupt
    gpio_config_t p = {
        .pin_bit_mask = 1ULL << PIR_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&p);

    // Install GPIO ISR service and register PIR ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIR_GPIO, pir_isr_handler, NULL);

    // Create PIR processing task pinned to core 1
    xTaskCreatePinnedToCore(
        pir_task,
        "pir_task",
        4096,
        NULL,
        5,
        NULL,
        1   // Core 1 – sensor processing
    );

    ESP_LOGI(TAG, "Detektor ruchu zainicjalizowany");
}
