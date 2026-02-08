#pragma once
// camera.h

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t camera_init(void);

void camera_resources_init(void);
