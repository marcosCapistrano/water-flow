idf_component_register(
    SRCS "storage.c" "wifi.c" "server.c" "main.c"
    INCLUDE_DIRS "." 
    REQUIRES driver esp_event esp_wifi vfs spiffs esp_http_server nvs_flash fatfs
    )

spiffs_create_partition_image(storage ../website/dist FLASH_IN_PROJECT)
