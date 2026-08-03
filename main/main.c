#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "io/gpio.h"
#include "io/adc.h"
#include "devices/ACS712.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    gpio_config_pin_t gpio = gpio_out_configure_pin(GPIO_NUM_24);
    adc_config_t acs712 = acs712_init(ADC_UNIT_1, ADC_CHANNEL_0);

    while (1) {
        int mAh = acs712_milliamperes(&acs712);
        if (mAh == ESP_FAIL) {
            ESP_LOGE(TAG, "ACS712 Failed to read data");
        }
        else {
            gpio_out_set_level(&gpio, mAh > 0);
        }


        vTaskDelay(pdMS_TO_TICKS(100));
    }

    adc_deinit(&acs712);
}
