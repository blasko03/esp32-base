#include "adc.h"
#include "esp_log.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "IO_ADC_ONESHOT";

bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


adc_config_t adc_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten)
{
    adc_config_t res = {
        .init = false,
        .channel = channel,
        .handle_oneshot = NULL,
        .cali_handle = NULL,
        .zerovolt = 0
    };

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
    };
    
    if (adc_oneshot_new_unit(&init_config, &res.handle_oneshot) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC unit");
        return res; 
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_12,
    };
    
    if (adc_oneshot_config_channel(res.handle_oneshot, channel, &channel_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC channel");
        return res; 
    }

    bool cali_success = adc_calibration_init(unit, channel, atten, &res.cali_handle);
    if (!cali_success) {
        res.cali_handle = NULL;
    }

    res.init = true;
    ESP_LOGI(TAG, "ADC Unit %d Channel %d configured", unit, channel);

    return res;
}


esp_err_t adc_read_raw(adc_config_t *config, int *out_value)
{
    if (config == NULL || out_value == NULL || !config->init) {
        return ESP_ERR_INVALID_STATE;
    }

    return adc_oneshot_read(config->handle_oneshot, config->channel, out_value);
}

esp_err_t adc_read_millivolts(adc_config_t *config, int *out_mv)
{
    if (config == NULL || out_mv == NULL || !config->init) {
        return ESP_ERR_INVALID_STATE;
    }

    int raw = 0;
    esp_err_t err = adc_oneshot_read(config->handle_oneshot, config->channel, &raw);
    if (err != ESP_OK) {
        return err;
    }

    if (config->cali_handle != NULL) {
        return adc_cali_raw_to_voltage(config->cali_handle, raw, out_mv);
    }

    *out_mv = raw; 
    return ESP_OK;
}

void adc_deinit(adc_config_t *config)
{
    if (config != NULL && config->init) {
        if (config->cali_handle != NULL) {
            #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
                adc_cali_delete_scheme_curve_fitting(config->cali_handle);
            #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
                adc_cali_delete_scheme_line_fitting(config->cali_handle);
            #endif
        }

        if (config->handle_oneshot != NULL) {
            adc_oneshot_del_unit(config->handle_oneshot);
        }

        config->init = false;
        config->handle_oneshot = NULL;
        config->cali_handle = NULL;
    }
}