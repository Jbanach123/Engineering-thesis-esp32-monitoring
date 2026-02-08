# pragma once
// config.h

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_camera.h"

// GPIO
#define PIR_GPIO   1   // PIR sensor
#define LED_GPIO   21  // LED 


#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"

#define SERVER_URL "http://192.168.137.1:5000/motion_detected?camera_id=cam2" // serwer Flask


extern SemaphoreHandle_t fb_mutex;
extern SemaphoreHandle_t camera_mutex;

extern bool motion_active;
extern int64_t last_motion_time;
extern int64_t last_event_time;

extern int64_t last_system_activity;
extern bool in_modem_sleep;

extern volatile int client_active;
extern uint8_t *last_jpeg_buf;
extern size_t last_jpeg_len;

extern uint8_t frames_per_second;
