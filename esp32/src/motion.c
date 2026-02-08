#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motion.h"
#include "config.h"

// Time after which motion is considered finished
#define MOTION_CLEAR_TIMEOUT_MS 10000
#define MOTION_TASK_DELAY_MS    200

static const char *TAG = "motion";

// Motion monitoring task
void motion_task(void *arg)
{
    while (1) {

        // Check if motion is currently active
        if (motion_active) {
            int64_t now = esp_timer_get_time() / 1000;

            // Clear motion state after inactivity timeout
            if (now - last_motion_time > MOTION_CLEAR_TIMEOUT_MS) { 
                motion_active = false;
                ESP_LOGI(TAG,
                    "Ruch zakończony (brak ruchu %lld ms)",
                    now - last_motion_time);
            }
        }

        // Periodic motion state evaluation
        vTaskDelay(pdMS_TO_TICKS(MOTION_TASK_DELAY_MS));
    }
}

// Start motion supervision task
void motion_start(void)
{
    // Create motion management task pinned to core 0
    xTaskCreatePinnedToCore(
        motion_task,
        "motion_task",
        2048,
        NULL,
        4,
        NULL,
        0
    );
}
