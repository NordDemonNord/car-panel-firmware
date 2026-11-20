idf_component_register(SRCS "main.c" "can_tx.c" "wifi_ap.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_twai
		       REQUIRES esp_wifi
		       REQUIRES nvs_flash)


