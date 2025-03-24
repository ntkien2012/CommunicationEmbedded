idf_component_register(SRCS "Wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_netif nvs_flash lwip)
