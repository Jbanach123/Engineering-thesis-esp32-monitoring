#include "esp_err.h"         
#include "esp_log.h"
#include "esp_camera.h"
#include "esp_http_server.h" 

#include <string.h> 
#include <stdlib.h> 

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "camera.h"
#include "config.h"


static const char *TAG = "camera";

// Pin definitions for the camera module
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13
 
// Camera configuration
static camera_config_t camera_config = {
    // Power and reset pins
    .pin_pwdn       = PWDN_GPIO_NUM,
    .pin_reset      = RESET_GPIO_NUM,

    // Clock and SCCB
    .pin_xclk       = XCLK_GPIO_NUM,
    .pin_sscb_sda   = SIOD_GPIO_NUM,
    .pin_sscb_scl   = SIOC_GPIO_NUM,

    // Data lines
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,

    // Sync signals
    .pin_vsync      = VSYNC_GPIO_NUM,
    .pin_href       = HREF_GPIO_NUM,
    .pin_pclk       = PCLK_GPIO_NUM,

    // XCLK configuration
    .xclk_freq_hz   = 10000000,
    .ledc_timer     = LEDC_TIMER_0,
    .ledc_channel   = LEDC_CHANNEL_0,

    // Image format configuration
    .pixel_format   = PIXFORMAT_JPEG,
    .frame_size     = FRAMESIZE_QVGA,
    .jpeg_quality   = 15,

    // Frame buffer configuration
    .fb_count       = 2,
    .fb_location    = CAMERA_FB_IN_PSRAM
};


// Camera resource initialization
void camera_resources_init(void)
{
    // Mutex protecting shared JPEG buffer
    fb_mutex = xSemaphoreCreateMutex();

    // Mutex protecting direct access to camera hardware
    camera_mutex = xSemaphoreCreateMutex();
}

// Camera initialization
esp_err_t camera_init(void)
{
    ESP_LOGI(TAG, "Inicjalizacja kamery...");
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return err;
    }
    ESP_LOGI(TAG, "Kamera zainicjalizowana poprawnie");
    return ESP_OK;
}
