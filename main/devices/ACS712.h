#include "io/adc.h"

adc_config_t acs712_init(adc_unit_t unit, adc_channel_t channel);
int acs712_milliamperes(adc_config_t *adc);