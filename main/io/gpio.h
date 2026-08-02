#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"
#include <stdbool.h>

typedef struct {
    gpio_num_t pin;
    bool init;
    gpio_mode_t mode;
    uint32_t level;
} gpio_config_pin_t;

gpio_config_pin_t gpio_out_configure_pin(gpio_num_t pin);
void gpio_out_set_level(gpio_config_pin_t *config, uint32_t level);

#endif 