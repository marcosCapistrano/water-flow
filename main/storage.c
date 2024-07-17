#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include "esp_spiffs.h"
#include "esp_log.h"

static const char *TAG = "STORAGE";

void storage_init()
{
    ESP_LOGI(TAG, "Inicializando SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/data",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    FILE *data_file = fopen("/data/readings.txt", "w");
    fprintf(data_file, "0,");
}

char *current_file = "/data/readings.txt";
void storage_write(int quantity)
{
        ESP_LOGI(TAG, "write");
    FILE *data_file = fopen(current_file, "a");
    if (data_file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(data_file, "%d,", quantity);
    fclose(data_file);
}

void storage_read()
{
        ESP_LOGI(TAG, "read");
    // Open renamed file for reading
    FILE *data_file = fopen("/data/readings.txt", "r");
    if (data_file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    ESP_LOGI(TAG, "Reading file");

    char buf[1024];
    int bytes_read = 0;
    while ((bytes_read = fread(buf, sizeof(char), sizeof(buf), data_file)) > 0)
    {
        printf("%.*s", bytes_read, buf);
    }
    fclose(data_file);
}