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
} ws_ctx_t;

// Connection state management
void net_evt_cb(net_evt_t evt, void *ctx);
static void ws_evt_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static ws_ctx_t _ctx;

status_t ws_init(char *uri, const config_network_t *net_config)
{
    strcpy(_ctx.uri, uri);
    _ctx.handler.cb = NULL;
    
    // Set up the network connection
    net_init(net_config);
    net_evt_cb_register(NET_EVT_CONNECT, (void *)&_ctx, net_evt_cb);
    net_evt_cb_register(NET_EVT_DISCONNECT, (void *)&_ctx, net_evt_cb);

    esp_log_level_set("websocket_client", ESP_LOG_WARN);

    return STATUS_OK;
}

status_t ws_start(void)
{
    return net_start();
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
    char *pkt = cJSON_PrintUnformatted(msg);
    DEBUG("sending: %s", pkt);
    esp_websocket_client_send_text(_ctx.client, pkt, strlen(pkt), portMAX_DELAY);
    return STATUS_OK;
}

status_t ws_connect(esp_websocket_client_handle_t *client, char *uri)
{
    esp_websocket_client_config_t ws_cfg = {
        .uri = "ws://192.168.5.209:8000/ws/access/door/743ee316a398",
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .reconnect_timeout_ms = 30000,
        .network_timeout_ms = 30000,
    };
    
    *client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(*client, WEBSOCKET_EVENT_ANY, ws_evt_cb, (void *)*client);
    esp_websocket_client_start(*client);
    return STATUS_OK;
}

status_t ws_disconnect(esp_websocket_client_handle_t client)
{
    esp_websocket_client_close(client, portMAX_DELAY);
    //esp_websocket_unregister_events(client, WEBSOCKET_EVENT_ANY, ws_evt_cb);
    esp_websocket_client_destroy(client);
    return STATUS_OK;
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
        if (_ctx.handler.cb != NULL)
        {
            _ctx.handler.cb(WS_OPEN, NULL, _ctx.handler.ctx);
        }
        break;

    case WEBSOCKET_EVENT_DISCONNECTED:
        INFO("Websocket Disconnected");
        ERROR("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) 
        {
            ERROR("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            ERROR("reported from tls stack", data->error_handle.esp_tls_stack_err);
            ERROR("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }
        if (_ctx.handler.cb != NULL)
        {
            _ctx.handler.cb(WS_CLOSE, NULL, _ctx.handler.ctx);
        }
        break;

    case WEBSOCKET_EVENT_DATA:
        //INFO("WEBSOCKET_EVENT_DATA");
        //INFO("Received opcode=%d", data->op_code);
        if (data->op_code == 0x2) 
        {
            ERROR("Unexpected binary data");
            //ESP_LOG_BUFFER_HEX("Received binary data", data->data_ptr, data->data_len);
        } 
        else if (data->op_code == 0x08 && data->data_len == 2) 
        {
            WARN("Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        } 
        else 
        {
            DEBUG("Received=%.*s", data->data_len, (char *)data->data_ptr);
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
        break;

    case WEBSOCKET_EVENT_FINISH:
        INFO("WEBSOCKET_EVENT_FINISH");
        break;
    }
}

void net_evt_cb(net_evt_t evt, void *ctx)
{
    ws_ctx_t *ws_ctx = (ws_ctx_t *) ctx;
    if (evt == NET_EVT_CONNECT)
    {
        ws_connect(&ws_ctx->client, ws_ctx->uri);
    }
    if (evt == NET_EVT_DISCONNECT)
    {
        // use this for something  
    }
}