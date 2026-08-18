
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "io/adc.h"
#include "io/adc_cont.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali_scheme.h"
#include <math.h>

#define READ_LEN                    3200

static const char *TAG = "IO_ADC_CONT";
static TaskHandle_t s_task_handle;


#define MAX_PARSED_SAMPLES (READ_LEN / SOC_ADC_DIGI_RESULT_BYTES)
static uint8_t result[READ_LEN] = {0};
static adc_continuous_data_t parsed_data[MAX_PARSED_SAMPLES];

adc_config_t adc_cont_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten)
{
    adc_config_t res = {
        .init = false,
        .channel = channel,
        .handle_oneshot = NULL,
        .cali_handle = NULL,
        .zerovolt = 0
    };


    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = READ_LEN*4,
        .conv_frame_size = READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &res.handle_continuous));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20*1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = 1;
    adc_pattern[0].atten = atten;
    adc_pattern[0].channel = channel & 0x7;
    adc_pattern[0].unit = unit;
    adc_pattern[0].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    ESP_LOGI(TAG, "adc_pattern[%d].atten is :%"PRIx8, 0, adc_pattern[0].atten);
    ESP_LOGI(TAG, "adc_pattern[%d].channel is :%"PRIx8, 0, adc_pattern[0].channel);
    ESP_LOGI(TAG, "adc_pattern[%d].unit is :%"PRIx8, 0, adc_pattern[0].unit);
    bool cali_success = adc_calibration_init(ADC_UNIT_1, adc_pattern[0].channel, atten, &res.cali_handle);
    if (!cali_success) {
        res.cali_handle = NULL;
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(res.handle_continuous, &dig_cfg));

    return res;
}


static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

void adc_cont_start(adc_config_t *config, AdcOnData on_data, AdcOnDataStarted on_data_start, AdcOnDataCompleted on_data_completed){
    esp_err_t ret;
    uint32_t ret_num = 0;

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(config->handle_continuous, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(config->handle_continuous));

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) {
            ret = adc_continuous_read(config->handle_continuous, result, READ_LEN, &ret_num, 0);
            if (ret == ESP_OK) {
                uint32_t num_parsed_samples = 0;
                esp_err_t parse_ret = adc_continuous_parse_data(config->handle_continuous, result, ret_num, parsed_data, &num_parsed_samples);
                if (parse_ret == ESP_OK) {                
                    uint32_t num_parsed_valid_samples = 0;
                    if(on_data_start != NULL)
                        on_data_start(num_parsed_samples);
                    for (int i = 0; i < num_parsed_samples; i++) {
                        if (parsed_data[i].valid) {
                            num_parsed_valid_samples++;
                            int reading = parsed_data[i].raw_data;
                            if (config->cali_handle != NULL) {
                                adc_cali_raw_to_voltage(config->cali_handle, parsed_data[i].raw_data, &reading);
                            }
                            if(on_data != NULL)
                                on_data(reading);

                        } else {
                            ESP_LOGW(TAG, "Invalid data [ADC%d_Ch%d_%"PRIu32"]",
                                     parsed_data[i].unit + 1,
                                     parsed_data[i].channel,
                                     parsed_data[i].raw_data);
                        }
                    }
                    if(on_data_completed != NULL)
                        on_data_completed(num_parsed_valid_samples);
                } else {
                    ESP_LOGE(TAG, "Data parsing failed: %s", esp_err_to_name(parse_ret));
                }

                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */
                vTaskDelay(1);
            } else if (ret == ESP_ERR_TIMEOUT) {
                //We try to read `EXAMPLE_READ_LEN` until API returns timeout, which means there's no available data
                break;
            }
        }
    }
}

void adc_cont_deinit(adc_config_t *config){
    if (config != NULL && config->init) {
        if (config->cali_handle != NULL) {
            #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
                adc_cali_delete_scheme_curve_fitting(config->cali_handle);
            #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
                adc_cali_delete_scheme_line_fitting(config->cali_handle);
            #endif
        }

        if (config->handle_continuous != NULL) {
            adc_continuous_stop(config->handle_continuous);
            adc_continuous_deinit(config->handle_continuous);
        }

        config->init = false;
        config->handle_oneshot = NULL;
        config->cali_handle = NULL;
    }
}
