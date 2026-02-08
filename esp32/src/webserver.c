#include "webserver.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "config.h"  // fb_mutex, client_active 
#include "sleep.h"
#include "camera.h"
#include "led.h"
#include "esp_timer.h"
#include <string.h>
#include <lwip/sockets.h>

static const char *TAG = "WebServer";
// Indicates whether MJPEG streaming is currently active
static bool s_streaming = false;
// Mutex to protect streaming state (single client only)
static SemaphoreHandle_t stream_mutex = NULL;

// Stream MJPEG handler (1 FPS)
esp_err_t mjpeg_1hz_handler(httpd_req_t *req) {

    // Checking global FPS value (used only in this file)
    if (frames_per_second < 1) frames_per_second = 1;
    if (frames_per_second > 10) frames_per_second = 10;   

    char part_header[128];
    esp_err_t res = ESP_OK;

        // Check if stream_mutex is initialized
        if (!stream_mutex) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    // Ensure only one client streams at a time
    xSemaphoreTake(stream_mutex, portMAX_DELAY);
    if (s_streaming) {
        xSemaphoreGive(stream_mutex);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    s_streaming = true;
    xSemaphoreGive(stream_mutex);

    // Set response type to multipart for MJPEG streaming
    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");

    // Active state to prevent sleep mode
    client_active = 1;
    sleep_notify_activity();
    led_on();
    ESP_LOGI(TAG, "Start streamu MJPEG");
    
    // Stream loop
    while (s_streaming) {

        // Check if client socket is still connected
        int sock = httpd_req_to_sockfd(req);
        if (sock < 0 || lwip_fcntl(sock, F_GETFL, 0) < 0) {
            ESP_LOGW(TAG, "Klient rozłączony, kończę stream...");
            break;
        }

        camera_fb_t *fb = NULL;
        // Safely capture a frame from the camera (protected by mutex)
        if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(300)) == pdTRUE) {
            fb = esp_camera_fb_get();
            xSemaphoreGive(camera_mutex);
        }

        if (!fb) {
            ESP_LOGE(TAG, "Błąd przechwycenia obrazu");
            httpd_resp_send_500(req);
            break;
        }
        // Update shared JPEG buffer with the latest frame
        if (xSemaphoreTake(fb_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (last_jpeg_buf) {
                free(last_jpeg_buf);
                last_jpeg_buf = NULL;
                last_jpeg_len = 0;
            }

            last_jpeg_buf = malloc(fb->len);
            if (last_jpeg_buf) {
                memcpy(last_jpeg_buf, fb->buf, fb->len);
                last_jpeg_len = fb->len;
            }

            xSemaphoreGive(fb_mutex);
        }
        // Return the frame buffer after use
        esp_camera_fb_return(fb);

        uint8_t *jpeg_buf = NULL;
        size_t jpeg_len = 0;

        // Retrieve the latest JPEG frame for streaming
        if (xSemaphoreTake(fb_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            jpeg_buf = last_jpeg_buf;
            jpeg_len = last_jpeg_len;
            xSemaphoreGive(fb_mutex);
        }

        if (!jpeg_buf || jpeg_len == 0) {
            ESP_LOGW(TAG, "Brak klatki do wysłania");
            break;
        }
        // Prepare MJPEG header
        snprintf(part_header, sizeof(part_header),
            "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
            jpeg_len);

        // Send JPEG image data
        res = httpd_resp_send_chunk(req, part_header, strlen(part_header));
        if (res != ESP_OK) break;

        // End of MJPEG frame
        res = httpd_resp_send_chunk(req,
                                    (const char *)jpeg_buf,
                                    jpeg_len);
        if (res != ESP_OK) break;
        res = httpd_resp_send_chunk(req, "\r\n", 2);
        if (res != ESP_OK) break;

        vTaskDelay(pdMS_TO_TICKS(1000/frames_per_second)); // 1 FPS
    }
    // Free last JPEG buffer after stream ends
    if (xSemaphoreTake(fb_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        if (last_jpeg_buf) {
            free(last_jpeg_buf);
            last_jpeg_buf = NULL;
            last_jpeg_len = 0;
        }
        xSemaphoreGive(fb_mutex);
    }
    // Reset streaming state and client activity flag
    xSemaphoreTake(stream_mutex, portMAX_DELAY);
    s_streaming = false;
    xSemaphoreGive(stream_mutex);
    client_active = 0;
    ESP_LOGI(TAG, "Stream zakończony");
    return res;
}

// Start the web server
httpd_handle_t start_webserver(void) {
    // Initialize stream mutex if not already done
    if (!stream_mutex) {
        stream_mutex = xSemaphoreCreateMutex();
    }
    // Configure HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.recv_wait_timeout = 1;
    config.send_wait_timeout = 1;
    config.lru_purge_enable = true;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {

        // Stream endpoint
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = mjpeg_1hz_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);

        ESP_LOGI(TAG, "HTTP serwer uruchomiony");
        return server;
    }

    ESP_LOGE(TAG, "Nie udało się uruchomić serwera HTTP");
    return NULL;
}
