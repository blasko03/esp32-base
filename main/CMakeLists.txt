idf_component_register(SRCS "api/http.c"
                            "devices/ACS712.c"
                            "io/gpio.c"
                            "io/adc.c"
                            "io/adc_cont.c"
                            "main.c" 
                            "network/wifi/wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt nvs_flash esp_adc esp_http_server)

