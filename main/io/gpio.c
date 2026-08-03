#include "gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPIO";

gpio_config_pin_t gpio_out_configure_pin(gpio_num_t pin)
{
    gpio_config_pin_t res = {
        .init = false,
        .pin = pin,
        .mode = GPIO_MODE_OUTPUT,
        .level = -1
    };

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = res.mode,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .hys_ctrl_mode = GPIO_HYS_SOFT_DISABLE
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        res.init = true;
        ESP_LOGI(TAG, "GPIO %d configured as output", pin);
    } else {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", pin, esp_err_to_name(err));
    }

    return res;
}

esp_err_t gpio_out_set_level(gpio_config_pin_t *config, uint32_t level)
{   
    if (config == NULL || !config->init || config->mode != GPIO_MODE_OUTPUT) {
        ESP_LOGE(TAG, "Attempted to set level on uninitialized or NULL GPIO config");
        return ESP_FAIL;
    }

    if (config->level == level) {
        return ESP_OK;
    }

    gpio_set_level(config->pin, level);
    config->level = level;
    return ESP_OK;
}
