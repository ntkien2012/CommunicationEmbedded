idf_component_register(SRCS "system.c"
                    INCLUDE_DIRS "include"
                    REQUIRES MAVLink AES256 Wifi Bit_Message HammingCode
                    PRIV_REQUIRES "esp_driver_gpio")
