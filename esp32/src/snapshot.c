#include <string.h>
#include <stdlib.h>

// ESP-IDF core includes
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "esp_http_client.h"

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "config.h"
#include "snapshot.h"
#include "sleep.h"

static const char *TAG = "snapshot";

// Mutex to protect snapshot sending
static SemaphoreHandle_t snapshot_mutex = NULL;

// Sending snapshot in case of motion detected
void send_motion_snapshot(void) {

    // Create mutex if not exists
    if (snapshot_mutex == NULL) {
        snapshot_mutex = xSemaphoreCreateMutex();
        if (snapshot_mutex == NULL) {
            ESP_LOGE(TAG, "Nie udało się utworzyć snapshot_mutex");
            return;
        }
    }

    // Prevent concurrent snapshot transmissions
    if (xSemaphoreTake(snapshot_mutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGW(TAG, "Snapshot już w trakcie - pomijam");
        return;
    }

    sleep_notify_activity();

    uint8_t *local_buf = NULL;
    size_t local_len = 0;
    camera_fb_t *fb_direct = NULL;
    
    // Try to get the last JPEG frame from the shared buffer
    if (xSemaphoreTake(fb_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        if (last_jpeg_buf && last_jpeg_len > 0) {
            local_len = last_jpeg_len;
            local_buf = (uint8_t *)malloc(local_len);
            if (local_buf) {
                memcpy(local_buf, last_jpeg_buf, local_len);
            }
        }
        xSemaphoreGive(fb_mutex);
    }

    // If no shared frame, capture directly from camera
    if (!local_buf) {
        if (xSemaphoreTake(camera_mutex, (TickType_t)300) == pdTRUE) {
            fb_direct = esp_camera_fb_get();
            if (fb_direct) {
                local_len = fb_direct->len;
                local_buf = (uint8_t *)malloc(local_len);
                if (local_buf) memcpy(local_buf, fb_direct->buf, local_len);
            }
            xSemaphoreGive(camera_mutex);
        } else {
            ESP_LOGW(TAG, "Camera busy, nie pobrano klatki");
        }
    }

    // Abort if no frame available
    if (!local_buf || local_len == 0) {
        ESP_LOGW(TAG, "Brak klatki do wysłania");
        if (fb_direct) esp_camera_fb_return(fb_direct);
        if (local_buf) free(local_buf);
        xSemaphoreGive(snapshot_mutex);
        return;
    }

    // Configure HTTP client for snapshot upload
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");
    
    // Open HTTP connection and send the JPEG data
    esp_err_t err = esp_http_client_open(client, local_len);
    if (err == ESP_OK) {
        int wlen = esp_http_client_write(client, (const char *)local_buf, local_len);

        if (wlen == (int)local_len) {
            ESP_LOGI(TAG, "Wysłano zdjęcie: %d bajtów", wlen);
        } else {
            ESP_LOGW(TAG, "Partial/failed write %d/%zu", wlen, local_len);
        }

        // Read HTTP response status code
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP status: %d", status);
    } else {
        ESP_LOGE(TAG, "Błąd połączenia HTTP: %s", esp_err_to_name(err));
    }

    // Cleanup HTTP client resources
    esp_http_client_cleanup(client);

    // Release camera frame and local buffer
    if (fb_direct) esp_camera_fb_return(fb_direct);
    if (local_buf) free(local_buf);

    // Release snapshot mutex
    xSemaphoreGive(snapshot_mutex);
}
