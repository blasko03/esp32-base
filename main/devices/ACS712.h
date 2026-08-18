#include "io/adc.h"

enum current_type_t {
  ACS712_AC,
  ACS712_DC
};

enum reading_type_t {
  ACS712_CONTINUOUS,
  ACS712_ONESHOT
};

typedef void (*Acs712ReadingComplete)(int);

typedef struct {
    adc_config_t adc;
    enum current_type_t current_type;
    enum reading_type_t reading_type;
} acs712_config;

acs712_config acs712_init(adc_unit_t unit, adc_channel_t channel, enum reading_type_t reading_type, enum current_type_t current_type);
int acs712_milliamperes(adc_config_t *adc);
int acs712_millivolts(adc_config_t *adc, int mah);
int acs712_millivolts_ac(adc_config_t *adc);
int acs712_millivolts_dc(adc_config_t *adc);
void acs712_start(acs712_config *config, Acs712ReadingComplete action);