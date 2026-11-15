idf_component_register(SRCS "main.c" "can_tx.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_twai)