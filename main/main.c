#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

#include "wifi.h"
#include "storage.h"
#include "server.h"

static const char *TAG = "MAIN";

#define EXAMPLE_PCNT_HIGH_LIMIT 100
#define EXAMPLE_PCNT_LOW_LIMIT -100

#define EXAMPLE_CHAN_GPIO_A 2
#define EXAMPLE_CHAN_GPIO_B 0

static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

pcnt_unit_config_t unit_config = {
    .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
    .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
};

pcnt_chan_config_t chan_config = {
    .edge_gpio_num = EXAMPLE_CHAN_GPIO_A,
    .level_gpio_num = EXAMPLE_CHAN_GPIO_B,
};

pcnt_unit_handle_t pcnt_unit = NULL;
pcnt_channel_handle_t pcnt_chan = NULL;

void app_main(void)
{
    // Inicializações necessárias para o wifi/servidor web
    wifi_init();
    storage_init();
    server_init();

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));

    ESP_LOGI("MAIN", "add watch points and register callbacks");
    int watch_points[] = {EXAMPLE_PCNT_LOW_LIMIT, EXAMPLE_PCNT_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++)
    {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
    }
    pcnt_event_callbacks_t cbs = {
        .on_reach = example_pcnt_on_reach,
    };
    QueueHandle_t queue = xQueueCreate(10, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

    pcnt_unit_enable(pcnt_unit);
    pcnt_unit_start(pcnt_unit);

    int total_count = 0;
    int pulse_count = 0;
    int event_count = 0;
    while (1)
    {
        if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(1000)))
        {
            ESP_LOGI("MAIN", "Watch point event, count: %d", event_count);
            total_count += EXAMPLE_PCNT_HIGH_LIMIT;
        }
        else
        {
            ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
            // ESP_LOGI("MAIN", "Total count: %d", total_count + pulse_count);
            // ESP_LOGI("MAIN", "To Liters: %f", (total_count + pulse_count) / 4.5);
            storage_write((total_count + pulse_count) / 4.5);
            storage_read();
        }
    }
}