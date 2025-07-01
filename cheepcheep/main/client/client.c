#include "client.h"
#include "ota_dfu.h"
#include "ws.h"
#include "net.h"
#include "log.h"

#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define CLIENT_CMD_HANDLER_MAX  10U

// The server starts to send unparsable messages when the period is 10s
#define CLIENT_PING_PERIOD      9U //s

// Number of websocket reconnection attempts to tolerate before resetting the 
// network
#define CLIENT_WS_FAIL_LIMIT    3U

// Event handlers
static void client_ping_timer_cb(TimerHandle_t xTimer);
static void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx);
static void net_evt_cb(net_evt_t evt, void *ctx);
status_t client_msg_handler(msg_t *msg);

// Helpers
void client_build_uri(device_type_t device, const char *url, uint8_t *mac, char *uri);
void client_reset_task(void *params);

typedef struct {
    const config_portal_t *config;
    TimerHandle_t ping_timer;
    client_cmd_handler_t handlers[CLIENT_CMD_HANDLER_MAX];
    int ws_fails;
    TaskHandle_t reset_task_handle;
} client_ctx_t;

static client_ctx_t _ctx;

status_t client_init(const config_client_t *config, device_type_t device_type)
{
    status_t status; 

    _ctx.config = &config->portal;
    _ctx.ws_fails = 0;

    if (strlen(_ctx.config->api_secret) == 0)
    {
        ERROR("API secret not provided, please set.");
        return -STATUS_BAD_CONFIG;
    }
    if (strlen(_ctx.config->ws_url) == 0)
    {
        ERROR("Websocket URL not provided, please set.");
        return -STATUS_BAD_CONFIG;
    }

    for (int i=0; i<CLIENT_CMD_HANDLER_MAX; i++)
    {
        _ctx.handlers[i] = NULL;
    }

    client_handler_register(client_msg_handler);

    // Create ping timer - sends periodic pings to host
    _ctx.ping_timer = xTimerCreate(
        "Ping_Timer", 
        pdMS_TO_TICKS(1000 * CLIENT_PING_PERIOD), 
        true, 
        NULL, 
        client_ping_timer_cb
    );
    if (_ctx.ping_timer == NULL) { return -STATUS_NOMEM; }

    // Build the uri for the websocket server
    status = net_init(&config->net);

    char url[128];
    uint8_t mac[6];
    net_get_mac(mac);
    client_build_uri(device_type, _ctx.config->ws_url, mac, url);

    status = ws_init(url);
    if (status != STATUS_OK) { return status; }

    ota_dfu_init(&config->dfu);

    ws_evt_cb_register(ws_evt_cb, (void *)&_ctx);
    net_evt_cb_register(NET_EVT_CONNECT, (void *)&_ctx, net_evt_cb);
    net_evt_cb_register(NET_EVT_DISCONNECT, (void *)&_ctx, net_evt_cb);

    // status led off

    return STATUS_OK;
}

status_t client_open(void)
{
    net_start();
    return STATUS_OK;
}

status_t client_handler_register(client_cmd_handler_t handler)
{
    for (int i=0; i<CLIENT_CMD_HANDLER_MAX; i++)
    {
        if (_ctx.handlers[i] == NULL)
        {
            _ctx.handlers[i] = handler;
            return STATUS_OK;
        }
    }
    return -STATUS_NOMEM;
}

status_t client_send_msg(msg_t *msg)
{
    status_t status;
    cJSON *root = cJSON_CreateObject();
    msg_to_cJSON(msg, root);
    status = ws_send(root);
    cJSON_Delete(root);
    return status;   
}

static void client_ping_timer_cb(TimerHandle_t xTimer)
{
    msg_t msg = {
        .type = MSG_PING,
    };
    client_send_msg(&msg);
}

static void net_evt_cb(net_evt_t evt, void *ctx)
{
    if (evt == NET_EVT_DISCONNECT)
    {
        ws_close();
    }
    if (evt == NET_EVT_CONNECT)
    {
        INFO("Network connected, starting websocket");
        ws_open();
    }
}

void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx)
{
    client_ctx_t *client_ctx = (client_ctx_t *)ctx;
    switch (evt)
    {
        case WS_OPEN: {
            // Websocket good now, we can stop counting consecutive failures
            client_ctx->ws_fails = 0;

            // Send authentication request
            msg_t msg = {
                .type = MSG_AUTHENTICATE,
                .authenticate.secret_key = (char *)client_ctx->config->api_secret,
            };
            client_send_msg(&msg);
            break;
        }

        case WS_CLOSE:
            // The websocket will attempt to automatically reconnect. 
            // We only need to intervene if this happens repeatedly.
            ERROR("Client lost websocket connection");
            client_ctx->ws_fails++;
            if (client_ctx->ws_fails > CLIENT_WS_FAIL_LIMIT)
            {
                // Run a task to reset the connection. These calls 
                // can't be done in this context, so a worker task 
                // is run
                ERROR("websocket reconnection has failed %d times, resetting connection", client_ctx->ws_fails);
                client_ctx->ws_fails = 0;
                xTaskCreate(client_reset_task, "Reset_Task", 4096, ctx, 5, &client_ctx->reset_task_handle);
            }

            break;

        case WS_MSG:
        {
            // Parse in-bound message
            msg_t msg;
            msg_from_cJSON(data, &msg);

            // Send message to handlers
            for (int i=0; i<CLIENT_CMD_HANDLER_MAX; i++)
            {
                if (_ctx.handlers[i] != NULL)
                {
                    if (_ctx.handlers[i](&msg) == STATUS_OK)
                    {
                        break;
                    }
                }
            }
            break;
        }
    }
}

status_t client_msg_handler(msg_t *msg)
{
    if (msg->type == MSG_PING)
    {
        DEBUG("Ping received");
        msg_t msg = {
            .type = MSG_PONG,
        };
        return client_send_msg(&msg);
    }
    if (msg->type == MSG_PONG)
    {
        DEBUG("Pong received");
        return STATUS_OK;
    }
    if (msg->type == MSG_AUTHORISED)
    {
        if (msg->authorised.authorised)
        {
            DEBUG("Device authorized, sending IP");
        
            msg_t msg = {
                .type = MSG_IP_ADDR,
                .ip_address.ip_address = net_get_ip(),
            };
            client_send_msg(&msg);

            // Start pinging the websocket server
            xTimerStart(_ctx.ping_timer, portMAX_DELAY);
        }
        else
        {
            ERROR("Device is not authorized");
        }    
        return STATUS_OK;
    }

    return -STATUS_INVALID;
}

void client_build_uri(device_type_t device, const char *url, uint8_t *mac, char *uri)
{
    char dev_str[32];
    char mac_str[32];

    switch(device)
    {
        case DEVICE_DOOR:
            strcpy(dev_str, "door");
            break;

        case DEVICE_INTERLOCK:
            strcpy(dev_str, "interlock");
            break;

        case DEVICE_VENDING:
            strcpy(dev_str, "memberbucks");
            break;
    }
    
    // Assemble the websocket uri
    sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    INFO("mac: %s", mac_str);
    sprintf(uri, "%s/%s/%s", url, dev_str, mac_str);
    INFO("url: %s", uri);
}

void client_reset_task(void *params)
{
    status_t status;
    client_ctx_t *ctx = (client_ctx_t *) params;

    INFO("Closing socket");
    status = ws_close();
    if (status != STATUS_OK) { ERROR("Couldn't stop the websocket"); }

    INFO("Stopping network");
    status = net_stop();
    if (status != STATUS_OK) { ERROR("Couldn't stop the network"); }

    INFO("Starting network");
    status = net_start();
    if (status != STATUS_OK) { ERROR("Couldn't start the network"); }

    vTaskDelete(ctx->reset_task_handle);
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}