#include "io/adc.h"
#include "io/adc_cont.h"
#include "esp_log.h"
#include <math.h>
#include "ACS712.h"
#include "freertos/FreeRTOS.h"

const int MAX_MILLIVOLT = 3300;
const int MAX_CURRENT = 20000;
static const char *TAG = "ADC712";

uint64_t sum_mv = 0;
uint64_t sum_sq_mv = 0;

void on_data(int reading){
    sum_mv += reading;
    sum_sq_mv += reading * reading;
}

void on_start(uint32_t n_samples){
    sum_mv = 0;
    sum_sq_mv = 0;
}

void on_completed(uint32_t processed_samples){
    double live_zero = (double)sum_mv / processed_samples;
    
    double mean_sq = (double)sum_sq_mv / processed_samples;
    
    double variance = mean_sq - (live_zero * live_zero);
    
    if (variance < 0.0) {
        variance = 0.0;
    }
    
    double rms_mv = sqrt(variance);
    
    int res = (int)round(rms_mv);
    ESP_LOGI(TAG, "ACS712 mesured %dW %dVrms %dmV", res*MAX_CURRENT/MAX_MILLIVOLT*2*230/1000, res, (int)(sum_mv/processed_samples));
}


int acs712_millivolts_dc(adc_config_t *adc) {
    int num_samples = 1000;
    int64_t millivolts_sum = 0;
    int millivolts;
    
    for (int i = 0; i < num_samples; i++) {
        esp_err_t err = adc_read_millivolts(adc, &millivolts);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC Read Failed");
            return -1;
        }
        
        millivolts_sum += millivolts;
    }
    
    return millivolts_sum/num_samples-adc->zerovolt;
}

int acs712_millivolts_ac(adc_config_t *adc) {
    int num_samples = 1000;
    int64_t sum_mv = 0;
    int64_t sum_sq_mv = 0;
    int millivolts;
    
    for (int i = 0; i < num_samples; i++) {
        esp_err_t err = adc_read_millivolts(adc, &millivolts);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC Read Failed");
            return -1;
        }
        
        sum_mv += millivolts;
        
        sum_sq_mv += (int64_t)millivolts * millivolts;
    }
    
    double live_zero = (double)sum_mv / num_samples;
    
    double mean_sq = (double)sum_sq_mv / num_samples;
    
    double variance = mean_sq - (live_zero * live_zero);
    
    if (variance < 0.0) {
        variance = 0.0;
    }
    
    double rms_mv = sqrt(variance);
    
    return (int)round(rms_mv);
}

int acs712_milliamperes(adc_config_t *adc) {
    int mv = acs712_millivolts_ac(adc);
    return mv * MAX_CURRENT / MAX_MILLIVOLT;
}

int acs712_millivolts(adc_config_t *adc, int mah) {
    return (mah)*MAX_MILLIVOLT/MAX_CURRENT;
}

acs712_config acs712_init(adc_unit_t unit, adc_channel_t channel, enum reading_type_t reading_type, enum current_type_t current_type) {
    adc_config_t adc;
    if(reading_type == ACS712_CONTINUOUS){
        adc = adc_cont_init(unit, channel, ADC_ATTEN_DB_12);
    }
    if(reading_type == ACS712_ONESHOT){
        adc = adc_init(unit, channel, ADC_ATTEN_DB_12);
    }

    acs712_config config = {
        .adc = adc,
        .current_type = reading_type,
        .reading_type = current_type,
    };
    config.adc.zerovolt = acs712_millivolts_dc(&config.adc);
    ESP_LOGI(TAG, "Zero voltage was set to %d", config.adc.zerovolt);

    return config;
}

void acs712_start(acs712_config *config, Acs712ReadingComplete action) {
    adc_cont_start(&config->adc, on_data, on_start, on_completed);
}
