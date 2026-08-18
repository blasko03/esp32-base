typedef void (*AdcOnData)(int);
typedef void (*AdcOnDataCompleted)(uint32_t);
typedef void (*AdcOnDataStarted)(uint32_t);


adc_config_t adc_cont_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten);

void adc_cont_start(adc_config_t *config, AdcOnData on_data, AdcOnDataStarted on_data_start, AdcOnDataCompleted on_data_completed);

void adc_cont_deinit(adc_config_t *config);