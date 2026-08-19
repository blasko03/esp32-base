#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "io/gpio.h"
#include "io/adc.h"
#include "io/adc_cont.h"
#include "devices/ACS712.h"
#include "esp_timer.h"
#include <math.h>

static const char *TAG = "MAIN";

gpio_config_pin_t gpio;


void reading(int res, void *user_data){
    gpio_out_set_level(&gpio, res > 200);
    ESP_LOGI(TAG, "ACS712 mesured %dmAh", res);
}

void app_main(void)
{
    gpio = gpio_out_configure_pin(GPIO_NUM_24);
    acs712_config acs712 = acs712_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_CHANNEL_1, ACS712_CONTINUOUS, ACS712_AC);
    acs712_start(&acs712, reading, NULL, 500);
}
