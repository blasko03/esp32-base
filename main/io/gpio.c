
#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio.h"

void gpio_out_configure_pin(gpio_num_t pin)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .hys_ctrl_mode = GPIO_HYS_SOFT_DISABLE
    };

    gpio_config(&io_conf);

    ESP_LOGI("GPIO", "GPIO %d configured as output.\n", pin);
}

void gpio_out_set_level(gpio_num_t pin_number, uint32_t level){
    gpio_set_level(pin_number, level);
}
