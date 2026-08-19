#include "io/adc.h"

enum current_type_t {
  ACS712_AC,
  ACS712_DC
};

enum reading_type_t {
  ACS712_CONTINUOUS,
  ACS712_ONESHOT
};

typedef void (*Acs712ReadingComplete)(int, void *user_data);

typedef struct {
    adc_config_t adc;
    adc_channel_t channel;
    adc_channel_t channel_calibration;
    int num_channles;
    enum current_type_t current_type;
    enum reading_type_t reading_type;
    Acs712ReadingComplete action;
    void *action_user_data;
    uint64_t sum_mv;
    uint64_t sum_sq_mv;
    uint64_t sum_max_mv;
} acs712_config;

acs712_config acs712_init(adc_unit_t unit, adc_channel_t channel, adc_channel_t channel_calibration, enum reading_type_t reading_type, enum current_type_t current_type); 
int acs712_read(acs712_config *config);
void acs712_start(acs712_config *config, Acs712ReadingComplete action, void *action_user_data, int delay);