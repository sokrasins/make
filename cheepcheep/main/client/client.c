#include "client.h"
#include "ws.h"
#include "net.h"
#include "log.h"

#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define CLIENT_CMD_HANDLER_MAX 10U

// The server starts to send unparsable messages when the period is 10s
#define CLIENT_PING_PERIOD 9U //s

// Event handlers
static void client_ping_timer_cb(TimerHandle_t xTimer);
void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx);
status_t client_msg_handler(msg_t *msg);

// Helpers
void client_build_uri(device_type_t device, char *url, uint8_t *mac, char *uri);

typedef struct {
    const config_portal_t *config;
    TimerHandle_t ping_timer;
    client_cmd_handler_t handlers[CLIENT_CMD_HANDLER_MAX];
} client_ctx_t;

static client_ctx_t _ctx;

status_t client_init(device_type_t device, const config_portal_t *portal_config, const config_network_t *net_config)
{
    _ctx.config = portal_config;

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
    if (_ctx.ping_timer == NULL)
    {
        return -STATUS_NOMEM;
    }

    // Build the uri for the websocket server
    char uri[128];
    uint8_t mac[6];
    net_get_mac(mac);
    client_build_uri(device, _ctx.config->ws_url, mac, uri);

    ws_init(uri, net_config);
    ws_evt_cb_register(ws_evt_cb, (void *)&_ctx);
    ws_start();

    // status led off

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

void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx)
{
    client_ctx_t *client_ctx = (client_ctx_t *)ctx;
    switch (evt)
    {
        case WS_OPEN: {
            msg_t msg = {
                .type = MSG_AUTHENTICATE,
                .authenticate.secret_key = client_ctx->config->api_secret,
            };
            client_send_msg(&msg);
            break;
        }

        case WS_CLOSE:
            // TODO: reconnect?
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
        INFO("Ping received");
        msg_t msg = {
            .type = MSG_PONG,
        };
        return client_send_msg(&msg);
    }
    if (msg->type == MSG_PONG)
    {
        INFO("Pong received");
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

void client_build_uri(device_type_t device, char *url, uint8_t *mac, char *uri)
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
    sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    sprintf(uri, "%s/%s/%s", url, dev_str, mac_str);
}