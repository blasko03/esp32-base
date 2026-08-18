#ifndef ADC_H
#define ADC_H

#include <stdbool.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"

typedef struct {
    adc_oneshot_unit_handle_t handle_oneshot;
    adc_continuous_handle_t handle_continuous;
    adc_cali_handle_t cali_handle;
    adc_channel_t channel;
    bool init;
    int zerovolt;
} adc_config_t;

adc_config_t adc_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten);

esp_err_t adc_read_raw(adc_config_t *config, int *out_value);

esp_err_t adc_read_millivolts(adc_config_t *config, int *out_mv);

bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

void adc_deinit(adc_config_t *config);

#endif