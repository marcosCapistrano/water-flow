#ifndef SERVER_H
#define SERVER_H

#include "freertos/FreeRTOS.h"

typedef struct {
    uint8_t id;
} ServerMessage;

typedef struct {
    QueueHandle_t queue;
} Server;

Server server_init();

#endif