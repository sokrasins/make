#include "ota_dfu.h"
#include "log.h"
#include "net.h"
#include "config.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SKIP_COMMON_NAME_CHECK

typedef struct {
    TaskHandle_t dfu_task_handle;
    net_evt_handle_t net_evt_handle;
    const config_dfu_t *config;
} dfu_ctx_t;

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void net_evt_cb(net_evt_t evt, void *ctx);
void dfu_task(void *params);

static dfu_ctx_t _ctx;

status_t ota_dfu_init(const config_dfu_t *config)
{
    _ctx.config = config;

    // Register connection CB with net
    _ctx.net_evt_handle = net_evt_cb_register(NET_EVT_CONNECT,(void *)&_ctx, net_evt_cb);

    return STATUS_OK;
}

void net_evt_cb(net_evt_t evt, void *ctx)
{
    // Run net task, ready to handle network state changes
    dfu_ctx_t *dfu_ctx = (dfu_ctx_t *) ctx;
    xTaskCreate(dfu_task, "DFU_Task", 8192, ctx, 2, &dfu_ctx->dfu_task_handle);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        DEBUG("HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        DEBUG("HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        DEBUG("HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        DEBUG("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        DEBUG("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        DEBUG("HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        DEBUG("HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        DEBUG("HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void dfu_task(void *params)
{
    dfu_ctx_t * ctx = (dfu_ctx_t *) params;

    // Set config
    esp_http_client_config_t config = {
        .url = ctx->config->url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
    };

#ifdef SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    // We no longer need the event, we only check DFU once
    net_evt_cb_deregister(ctx->net_evt_handle);

    INFO("Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);

    if (ret == ESP_OK) 
    {
        INFO("OTA Succeed, Rebooting...");
        esp_restart();
    } 
    else
    {
        ERROR("Firmware upgrade failed: %d", ret);
    }

    // Kill the task, it is NOT allowed to return
    vTaskDelete(ctx->dfu_task_handle);
}