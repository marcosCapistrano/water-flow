#ifndef SENSOR_H
#define SENSOR_H

#include "freertos/FreeRTOS.h"
#include "driver/pulse_cnt.h"


typedef struct {
    QueueHandle_t queue;
    pcnt_unit_handle_t unit;
} Sensor;

void sensor_init();
void sensor_start();
void sensor_stop();

#endif