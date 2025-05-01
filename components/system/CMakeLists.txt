idf_component_register(SRCS "system.c"
                    INCLUDE_DIRS "include"
                    REQUIRES MAVLink AES256 Wifi Bit_Message LDPC
                    PRIV_REQUIRES "esp_driver_gpio" "esp_driver_uart")
