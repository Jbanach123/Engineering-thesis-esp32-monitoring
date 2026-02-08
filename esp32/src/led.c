#include "led.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "led";

// Initialize status LED GPIO
void led_init(void)
{   
    // Configure LED GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
    led_on(); 

    ESP_LOGI(TAG, "LED initialized on GPIO %d", LED_GPIO);
}

// Turn status LED on (active LOW)
void led_on(void)
{
    gpio_set_level(LED_GPIO, 0);
}

void led_off(void)
{
    gpio_set_level(LED_GPIO, 1);
}

