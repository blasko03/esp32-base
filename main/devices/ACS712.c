#include "io/adc.h"
#include "io/adc_cont.h"
#include "esp_log.h"
#include <math.h>
#include "ACS712.h"
#include "freertos/FreeRTOS.h"

static const int ORIGINAL_VOLTAGE = 5000;
static const int SCALED_VOLTAGE = 3300;
static const  int MIN_MILLIVOLT = 500;
static const int MAX_MILLIVOLT = 4500;
static const int MAX_CURRENT = 20000;
static const int N_SAMPLES = 800;
static const char *TAG = "ADC712";

int to_miliamperes(int mV, int mv_ref) {
    ESP_LOGI(TAG, "mV reading %d %d", mV, mv_ref);
    int max_millivolt = MAX_MILLIVOLT*mv_ref/ORIGINAL_VOLTAGE;
    int min_millivolt = MIN_MILLIVOLT*mv_ref/ORIGINAL_VOLTAGE;

    return MAX_CURRENT+((mV-max_millivolt)*MAX_CURRENT*2/(max_millivolt-min_millivolt));
}

void on_data(int reading, adc_channel_t channel, void *user_data){
    acs712_config *config = (acs712_config *)user_data;
    if(channel == config->channel) {
        config->sum_mv += reading;
        config->sum_sq_mv += reading * reading;
    }

    if(channel == config->channel_calibration) {
        config->sum_max_mv += reading;
    }
}

void on_start(uint32_t n_samples, void *user_data){
    acs712_config *config = (acs712_config *)user_data;
    config->sum_mv = 0;
    config->sum_sq_mv = 0;
    config->sum_max_mv = 0;
}

int on_completed(uint32_t processed_samples, void *user_data){
    acs712_config *config = (acs712_config *)user_data;
    int samples_per_channel = processed_samples/config->num_channles;
    int mv_ref=config->channel_calibration == -1 ? SCALED_VOLTAGE : config->sum_max_mv/samples_per_channel;
    int max_millivolt = MAX_MILLIVOLT*mv_ref/ORIGINAL_VOLTAGE;
    int min_millivolt = MIN_MILLIVOLT*mv_ref/ORIGINAL_VOLTAGE;
    int total;
    if(config->current_type == ACS712_DC){
        total = to_miliamperes(((int)config->sum_mv / samples_per_channel), mv_ref);
    }
    else {
        double live_zero = (double)config->sum_mv / samples_per_channel;
        
        double mean_sq = (double)config->sum_sq_mv / samples_per_channel;
        
        double variance = mean_sq - (live_zero * live_zero);
        
        if (variance < 0.0) {
            variance = 0.0;
        }

        total = to_miliamperes((max_millivolt+min_millivolt)/2+(int)round(sqrt(variance)), mv_ref);
    }
    if(config->action)
        config->action(total, config->action_user_data);
    return total;
}


int acs712_read(acs712_config *config) {
    int millivolts;
    on_start(N_SAMPLES*config->num_channles, config);
    
    for (int i = 0; i < N_SAMPLES; i++) {
        esp_err_t err = adc_read_millivolts(&config->adc, config->channel, &millivolts);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADC Read Failed");
            return ESP_FAIL;
        }
        on_data(millivolts, config->channel, config);
    }

    if(config->channel_calibration != -1){
        for (int i = 0; i < N_SAMPLES; i++) {
            esp_err_t err = adc_read_millivolts(&config->adc, config->channel_calibration, &millivolts);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "ADC Read Failed");
                return ESP_FAIL;
            }

            on_data(millivolts, config->channel_calibration, config);
        }
    }

    return on_completed(N_SAMPLES*config->num_channles, config);
}

acs712_config acs712_init(adc_unit_t unit, adc_channel_t channel, adc_channel_t channel_calibration, enum reading_type_t reading_type, enum current_type_t current_type) {
    adc_config_t adc;
    int num_channles = channel_calibration == -1 ? 1 : 2;
    adc_channel_t channels[2] = {channel, channel_calibration};
    if(reading_type == ACS712_CONTINUOUS){
        adc = adc_cont_init(unit, channels, num_channles, ADC_ATTEN_DB_12, N_SAMPLES);
    }
    if(reading_type == ACS712_ONESHOT){
        adc = adc_init(unit, channels, num_channles, ADC_ATTEN_DB_12);
    }

    acs712_config config = {
        .adc = adc,
        .channel = channel,
        .channel_calibration = channel_calibration,
        .current_type = current_type,
        .reading_type = reading_type,
        .num_channles = num_channles
    };

    return config;
}

void acs712_start(acs712_config *config, Acs712ReadingComplete action, void *action_user_data, int delay) {
    if(config->reading_type == ACS712_CONTINUOUS) {
        config->action = action;
        config->action_user_data = action_user_data;
        adc_cont_start(&config->adc, on_data, on_start, on_completed, (void *)config);
    } else if(config->reading_type == ACS712_ONESHOT){
        while(1) {
            int res = acs712_read(config);
            if(res != ESP_FAIL) {
                action(res, action_user_data);
            }
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}
