#include "nvs_flash.h"

#include "snapshot.h"
#include "pir.h"
#include "config.h"
#include "wifi.h"
#include "webserver.h"
#include "camera.h"
#include "sleep.h"
#include "motion.h"
#include "led.h"

void app_main(void)
{

    // Initialize status LED (system state indicator)
    led_init();

    // Initialize PIR motion sensor (GPIO, ISR, and task)
    pir_init();

    // Initialize camera resources (mutexes)
    camera_resources_init();

    // Initialize non-volatile storage (required by Wi-Fi and networking)
    // Abort system execution if initialization fails
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize Wi-Fi subsystem (STA mode with static IP)
    wifi_init_sta();

    // Initialize camera hardware and driver
    ESP_ERROR_CHECK(camera_init());

    // Start HTTP server (MJPEG streaming and snapshot endpoint)
    start_webserver();
     
    // Start motion detection supervision task
    motion_start();

    // Start sleep manager task (MODEM_SLEEP)
    sleep_manager_init();
}