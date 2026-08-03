#include "io/adc.h"
#include "esp_log.h"

const int MAX_MILLIVOLT = 2500;
const int MAX_CURRENT = 20000;


adc_config_t acs712_init(adc_unit_t unit, adc_channel_t channel) {
    return adc_init(unit, channel, ADC_ATTEN_DB_12);
}

int acs712_milliamperes(adc_config_t *adc) {
    int millivolts;
    esp_err_t err = adc_read_millivolts(adc, &millivolts);
    if (err != ESP_OK) {
        ESP_LOGI("MAIN", "ADC Raw Value: %d", millivolts);
        return ESP_FAIL;
    }
    return millivolts*MAX_CURRENT/MAX_MILLIVOLT;
}
