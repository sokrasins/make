#include "ws.h"
#include "net.h"
#include "log.h"

#include "esp_websocket_client.h"
#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include <stddef.h>
#include <string.h>

typedef struct {
    ws_evt_cb_t cb;
    void *ctx;
} ws_evt_handler_t;

typedef struct {
    esp_websocket_client_handle_t client;
    char uri[64];
    ws_evt_handler_t handler;
    bool connected;
} ws_ctx_t;

// Connection state management
static void ws_evt_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static ws_ctx_t _ctx;

status_t ws_init(char *url)
{
    //status_t status;

    _ctx.handler.cb = NULL;
    _ctx.connected = false;

    esp_log_level_set("websocket_client", ESP_LOG_DEBUG);

    strcpy(_ctx.uri, url);

    esp_websocket_client_config_t ws_cfg = {
        .uri = _ctx.uri,                    // Server uri
        .skip_cert_common_name_check = true,// For now skip verification, but this should change in production
        .crt_bundle_attach = esp_crt_bundle_attach, // Basic cert bundle for most common 
        .reconnect_timeout_ms = 10000,      // Default
        .network_timeout_ms = 10000,        // Default
        .ping_interval_sec = 0xFFFFFFFF,    // Disable the automated ping, the server expects a client-level ping-pong
    };

    _ctx.client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(_ctx.client, WEBSOCKET_EVENT_ANY, ws_evt_cb, (void *)&_ctx.client);

    return STATUS_OK;
}

status_t ws_open(void)
{
    esp_websocket_client_start(_ctx.client);
    return STATUS_OK;
}

status_t ws_close(void)
{
    esp_websocket_client_close(_ctx.client, pdMS_TO_TICKS(3000));
    return STATUS_OK;
}

status_t ws_evt_cb_register(ws_evt_cb_t cb, void *ctx)
{
    if (_ctx.handler.cb == NULL)
    {
        _ctx.handler.cb = cb;
        _ctx.handler.ctx = ctx;
        return STATUS_OK;
    }

    return -STATUS_NOMEM;
}

status_t ws_send(cJSON *msg)
{
    if (_ctx.connected)
    {
        char *pkt = cJSON_PrintUnformatted(msg);
        DEBUG("sending: %s", pkt);
        esp_websocket_client_send_text(_ctx.client, pkt, strlen(pkt), portMAX_DELAY);
        return STATUS_OK;
    }
    return -STATUS_NO_RESOURCE;
}

static void ws_evt_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) 
    {
    case WEBSOCKET_EVENT_BEGIN:
        break;

    case WEBSOCKET_EVENT_CONNECTED:
        INFO("Websocket Connected");
        _ctx.connected = true;
        if (_ctx.handler.cb != NULL)
        {
            _ctx.handler.cb(WS_OPEN, NULL, _ctx.handler.ctx);
        }
        break;

    case WEBSOCKET_EVENT_DISCONNECTED:
        INFO("Websocket Disconnected");
        _ctx.connected = false;
        ERROR("HTTP status code: %d",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) 
        {
            ERROR("reported from esp-tls: %d", data->error_handle.esp_tls_last_esp_err);
            ERROR("reported from tls stack: %d", data->error_handle.esp_tls_stack_err);
            ERROR("captured as transport's socket errno: %d",  data->error_handle.esp_transport_sock_errno);
        }
        if (_ctx.handler.cb != NULL)
        {
            _ctx.handler.cb(WS_CLOSE, NULL, _ctx.handler.ctx);
        }
        break;

    case WEBSOCKET_EVENT_DATA:
        if (data->op_code == 0x2) 
        {
            ERROR("Unexpected binary data");
        } 
        else if (data->op_code == 0x08 && data->data_len == 2) 
        {
            WARN("Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        } 
        else 
        {
            INFO("Received=%.*s", data->data_len, (char *)data->data_ptr);
        }

        // Try to parse a json payload. If we succeed, then send it to be parsed further.
        cJSON *msg = cJSON_Parse(data->data_ptr);
        if (msg) 
        {
            if (_ctx.handler.cb != NULL)
            {
                _ctx.handler.cb(WS_MSG, msg, _ctx.handler.ctx);
            }
            cJSON_Delete(msg);
        }
        break;

    case WEBSOCKET_EVENT_ERROR:
        INFO("Websocket Error");
        ERROR("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) 
        {
            ERROR("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            ERROR("reported from tls stack", data->error_handle.esp_tls_stack_err);
            ERROR("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }

        //sys_restart();
        break;

    case WEBSOCKET_EVENT_FINISH:
        INFO("WEBSOCKET_EVENT_FINISH");
        // TODO: event here
        // this is sent from the server when the device successfully 
        // connects, but isn't authorized.
        break;
    }
}
