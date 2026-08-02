idf_component_register(SRCS "main.c" 
                            "devices/wifi/wifi.c"
                            "devices/bluetooth/gatt_svc.c"
                            "devices/bluetooth/gap.c"
                            "io/gpio.c"
                    INCLUDE_DIRS "." "devices" "io"
                    REQUIRES bt nvs_flash)

