idf_component_register(SRCS "motion.c" "led.c" "sleep.c" "camera.c" "webserver.c" "wifi.c" "config.c" "pir.c" "snapshot.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi nvs_flash esp_http_server esp_http_client esp32-camera esp_timer)
