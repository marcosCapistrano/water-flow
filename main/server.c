#include "server.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "sensor.h"
#include "storage.h"

static const char *TAG = "SERVER";

static httpd_handle_t server_handle = NULL;
int id = 1;

extern Server server;
extern Sensor sensor;

static esp_err_t on_sensor_toggle(httpd_req_t *req)
{
    sensor_start();
    ESP_LOGI(TAG, "Called sensor toggle");

    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

static esp_err_t on_readings(httpd_req_t *req)
{

    FILE *file = fopen("/data/readings.txt", "r");
    if (!file)
    {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char buf[1024];
    size_t bytes_read;

    // Prepare SVG header
    const char *svgHeader = "<svg width='800' height='600' xmlns='http://www.w3.org/2000/svg'>\n<rect width='100\\%' height='100\\%' fill='white' />\n";
    httpd_resp_send_chunk(req, svgHeader, strlen(svgHeader));

    int y;
    int xPos = 50; // Initial X position
    const int yOffset = 300; // Center Y position
    const int xStep = 1; // Step size for X-axis
    int prevX = 50, prevY = yOffset; // Initial previous point

    while (fscanf(file, "%d,", &y)>0) {
        int yPos = yOffset - y;

        // Draw the line in SVG
        int len = snprintf(buf, sizeof(buf), "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke='blue' stroke-width='2'/>\n", prevX, prevY, xPos, yPos);
        httpd_resp_send_chunk(req, buf, len);

        // Update previous points
        prevX = xPos;
        prevY = yPos;
        xPos += xStep;
    }

    // Prepare SVG footer
    const char *svgFooter = "</svg>\n";
    httpd_resp_send_chunk(req, svgFooter, strlen(svgFooter));

    // Send final chunk to indicate the end of the response
    httpd_resp_send_chunk(req, NULL, 0);

    fclose(file);

    return ESP_OK;
}

static esp_err_t on_default_url(httpd_req_t *req)
{
    ESP_LOGI(TAG, "URL: %s", req->uri);
    char path[600];
    sprintf(path, "/data%s", req->uri);

    char *ext = strrchr(req->uri, '.');
    if (ext != NULL)
    {
        if (strcmp(ext, ".css") == 0)
            httpd_resp_set_type(req, "text/css");
        if (strcmp(ext, ".js") == 0)
            httpd_resp_set_type(req, "text/javascript");
        if (strcmp(ext, ".js") == 0)
            httpd_resp_set_type(req, "text/javascript");
    }

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        file = fopen("/data/index.html", "r");
        if (file == NULL)
        {
            httpd_resp_send_404(req);
        }
        // handle erorr
        ESP_LOGE(TAG, "Failed to open index.html");
    }

    char buf[1024];
    int bytes_read = 0;
    while ((bytes_read = fread(buf, sizeof(char), sizeof(buf), file)) > 0)
    {
        httpd_resp_send_chunk(req, buf, bytes_read);
    }
    httpd_resp_send_chunk(req, NULL, 0);

    fclose(file);

    return ESP_OK;
}

static const httpd_uri_t wildcard = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = on_default_url,
    .user_ctx = NULL};

static const httpd_uri_t contact = {
    .uri = "/leitura/toggle",
    .method = HTTP_GET,
    .handler = on_sensor_toggle,
    .user_ctx = NULL};

static const httpd_uri_t readings = {
    .uri = "/data/readings",
    .method = HTTP_GET,
    .handler = on_readings,
    .user_ctx = NULL};

Server server_init()
{
    Server server;
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    /* Start the httpd server */
    if (httpd_start(&server_handle, &config) == ESP_OK)
    {
        /* Register URI handlers */
        httpd_register_uri_handler(server_handle, &contact);
        httpd_register_uri_handler(server_handle, &readings);
        httpd_register_uri_handler(server_handle, &wildcard);
    }
    /* If server failed to start, handle will be NULL */

    server.queue = xQueueCreate(10, sizeof(int));
    if (server.queue == 0)
    {
        ESP_LOGE(TAG, "Falha na criação da queue do servidor");
    }

    sensor_stop();

    return server;
}