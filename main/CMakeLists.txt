idf_component_register(SRCS "main.c" 
                            "network/wifi/wifi.c"
                            "io/gpio.c"
                            "io/adc.c"
                            "io/adc_cont.c"
                            "devices/ACS712.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt nvs_flash esp_adc)

