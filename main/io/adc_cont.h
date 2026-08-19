typedef void (*AdcOnData)(int, adc_channel_t, void*);
typedef int (*AdcOnDataCompleted)(uint32_t, void*);
typedef void (*AdcOnDataStarted)(uint32_t, void*);


adc_config_t adc_cont_init(adc_unit_t unit, adc_channel_t *channel, uint8_t channel_num, adc_atten_t atten);

void adc_cont_start(adc_config_t *config, AdcOnData on_data, AdcOnDataStarted on_data_start, AdcOnDataCompleted on_data_completed, void *user_data);

void adc_cont_deinit(adc_config_t *config);