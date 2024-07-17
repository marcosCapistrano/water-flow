#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include "esp_log.h"

#include "wifi.h"
#include "storage.h"
#include "server.h"
#include "sensor.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // Inicializações necessárias para o wifi/servidor web
    wifi_init();
    storage_init();
    sensor_init();
    server_init();
}