#pragma once
//webserver.h

#include "esp_http_server.h"
#include "esp_err.h"

httpd_handle_t start_webserver(void);
