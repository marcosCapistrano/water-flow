#include "esp_log.h"

#include "sensor.h"
#include "storage.h"

#define PCNT_HIGH_LIMIT 100
#define PCNT_LOW_LIMIT -100

#define CHAN_GPIO_A 2

Sensor sensor;

static const char *TAG = "SENSOR";

static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

pcnt_unit_config_t unit_config = {
    .high_limit = PCNT_HIGH_LIMIT,
    .low_limit = PCNT_LOW_LIMIT,
};

pcnt_chan_config_t chan_config = {
    .edge_gpio_num = CHAN_GPIO_A,
};

void sensor_init()
{
    pcnt_channel_handle_t pcnt_chan = NULL;

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &sensor.unit));
    ESP_ERROR_CHECK(pcnt_new_channel(sensor.unit, &chan_config, &pcnt_chan));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));

    int watch_points[] = {PCNT_LOW_LIMIT, PCNT_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++)
    {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(sensor.unit, watch_points[i]));
    }
    pcnt_event_callbacks_t cbs = {
        .on_reach = example_pcnt_on_reach,
    };

    sensor.queue = xQueueCreate(10, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(sensor.unit, &cbs, sensor.queue));

    ESP_ERROR_CHECK(pcnt_unit_enable(sensor.unit));
}

void vTaskSensor()
{
    int sum = 0;
    for (;;)
    {
        ESP_LOGI(TAG, "Task running");
        int pulse_count = 0;
        ESP_ERROR_CHECK(pcnt_unit_get_count(sensor.unit, &pulse_count));
        sum += pulse_count;

        storage_write(sum);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

TaskHandle_t xHandle = NULL;
BaseType_t xReturned = NULL;

bool started = false;
void sensor_start()
{
    if (!started)
    {
        ESP_LOGI(TAG, "start");
        pcnt_unit_start(sensor.unit);

        xReturned = xTaskCreate(
            vTaskSensor,      /* Function that implements the task. */
            "NAME",           /* Text name for the task. */
            4028,             /* Stack size in words, not bytes. */
            (void *)NULL,     /* Parameter passed into the task. */
            tskIDLE_PRIORITY, /* Priority at which the task is created. */
            &xHandle);        /* Used to pass out the created task's handle. */
            started = true;
    }
    else
    {
        sensor_stop();
        started = false;
    }
}

void sensor_stop()
{
    ESP_LOGI(TAG, "stop");
    vTaskDelete(xHandle);
    storage_read();
    pcnt_unit_stop(sensor.unit);
    pcnt_unit_clear_count(sensor.unit);
}