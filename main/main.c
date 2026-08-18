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

void app_main(void)
{
    gpio = gpio_out_configure_pin(GPIO_NUM_24);
    /*acs712_config acs712 = acs712_init(ADC_UNIT_1, ADC_CHANNEL_0, ONESHOT, AC);

    while (1) {
        int64_t start_time = esp_timer_get_time();
        int mVAC = acs712_millivolts_ac(&acs712.adc);
        int64_t end_time = esp_timer_get_time(); 
        ESP_LOGI(TAG, "acs712_millivolts_ac did take %lld microseconds", end_time - start_time);

        if (mVAC == ESP_FAIL) {
            ESP_LOGE(TAG, "ACS712 Failed to read data");
        }
        else {
            ESP_LOGI(TAG, "ACS712 mesured mVAC %d", mVAC);
            gpio_out_set_level(&gpio, mVAC > 60);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    adc_deinit(&acs712.adc);*/

    acs712_config acs712 = acs712_init(ADC_UNIT_1, ADC_CHANNEL_0, ACS712_CONTINUOUS, ACS712_AC);
    acs712_start(&acs712, NULL);
}
