#include <string.h>
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
}

char *current_file = NULL;

void storage_write(double quantity)
{
    if (current_file != NULL)
    {
        FILE *data_file = fopen(current_file, "a");
        if (data_file == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            return;
        }

        fprintf(data_file, "%f,", quantity);
        fclose(data_file);
    } else {
        // create_new_file("");
    }
}

void storage_read()
{
    // Open renamed file for reading
    FILE *data_file = fopen("/data/data.txt", "r");
    if (data_file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    ESP_LOGI(TAG, "Reading file");
    char line[640];
    fgets(line, sizeof(line), data_file);
    fclose(data_file);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}