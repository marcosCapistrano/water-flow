#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "SERVER";

static httpd_handle_t server = NULL;

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

void server_init()
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK)
    {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &wildcard);
    }
    /* If server failed to start, handle will be NULL */
}