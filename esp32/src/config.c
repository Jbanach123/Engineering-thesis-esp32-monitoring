#include "config.h"

// **Camera variables**
// Mutex protecting access to the shared JPEG frame buffer
SemaphoreHandle_t fb_mutex = NULL;
// Mutex protecting camera access
SemaphoreHandle_t camera_mutex = NULL;

// **Motion variables**
bool motion_active = false;
int64_t last_motion_time = 0;
// Timestamp of the last motion-triggered action (cooldown handling)
int64_t last_event_time = 0;

// **Sleep variables**
int64_t last_system_activity = 0;
bool in_modem_sleep = false;

// **Web variables**
volatile int client_active;
// Buffer holding the last captured JPEG frame (shared between tasks)
uint8_t *last_jpeg_buf = NULL;
// Size of the last captured JPEG frame
size_t last_jpeg_len = 0;

// MJPEG stream frame rate (FPS)
uint8_t frames_per_second = 1;   // default: 1 FPS
