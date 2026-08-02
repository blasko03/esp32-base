#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "devices/wifi/wifi.h"

//#include "devices/bluetooth/common.h"
//#include "devices/bluetooth/gap.h"
//#include "devices/bluetooth/gatt_svc.h"
#include "io/gpio.h"

void app_main(void)
{
    init_wifi();

    gpio_out_configure_pin(GPIO_NUM_1);
    gpio_out_set_level(GPIO_NUM_1, 1);
}
