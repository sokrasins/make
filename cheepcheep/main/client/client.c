#include "client.h"
#include "ws.h"
#include "net.h"
#include "log.h"

#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define CLIENT_CMD_HANDLER_MAX 10U

#define CLIENT_PING_PERIOD 9U //s

// Client management
static void client_ping_timer_cb(TimerHandle_t xTimer);
void client_handle_inbound_msg(cJSON *msg_in);
status_t client_send_msg(msg_type_t msg_type, cJSON *root);
void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx);
void client_build_uri(device_type_t device, char *url, uint8_t *mac, char *uri);

// Local command handling
status_t ws_cmd_handler(msg_type_t msg_type, cJSON *payload);
status_t send_authenticate(const char* api_secret);
status_t send_ipaddr(uint32_t ip);
status_t send_ping(void);
status_t send_pong(void);

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

    client_handler_register(ws_cmd_handler);

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

status_t client_send_msg(msg_type_t msg_type, cJSON *root)
{
    cJSON_AddStringToObject(root, "command", msgtype_to_str(msg_type));
    ws_send(root);
    return STATUS_OK;
}

static void client_ping_timer_cb(TimerHandle_t xTimer)
{
    send_ping();
}

status_t send_authenticate(const char* api_secret)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "secret_key", cJSON_CreateString(api_secret));
    status_t status = client_send_msg(MSG_AUTHENTICATE, root);
    cJSON_Delete(root);
    return status;
}

status_t send_ipaddr(uint32_t ip)
{
    char ip_str[32];
    sprintf(ip_str, "%u.%u.%u.%u", 
        (uint8_t) ((ip >> 0) & 0xFF), 
        (uint8_t) ((ip >> 8) & 0xFF), 
        (uint8_t) ((ip >> 16) & 0xFF), 
        (uint8_t) ((ip >> 24) & 0xFF)
    );

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "ip_address", cJSON_CreateString(ip_str));
    status_t status = client_send_msg(MSG_IP_ADDR, root);
    cJSON_Delete(root);
    return status;
}

status_t send_ping(void)
{
    cJSON *root = cJSON_CreateObject();
    status_t status = client_send_msg(MSG_PING, root);
    cJSON_Delete(root);
    return status;
}

status_t send_pong(void)
{
    cJSON *root = cJSON_CreateObject();
    status_t status = client_send_msg(MSG_PONG, root);
    cJSON_Delete(root);
    return status;
}

void ws_evt_cb(ws_evt_t evt, cJSON *data, void *ctx)
{
    client_ctx_t *client_ctx = (client_ctx_t *)ctx;
    switch (evt)
    {
        case WS_OPEN:
            send_authenticate(client_ctx->config->api_secret);
            break;

        case WS_CLOSE:

            break;

        case WS_MSG:
            client_handle_inbound_msg(data);
            break;
    }
}

status_t ws_cmd_handler(msg_type_t msg_type, cJSON *payload)
{
    if (msg_type == MSG_PING)
    {
        INFO("Ping received");
        send_pong();
        return STATUS_OK;
    }
    if (msg_type == MSG_PONG)
    {
        INFO("Pong received");
        return STATUS_OK;
    }
    if (msg_type == MSG_AUTHORISED)
    {
        INFO("Device authorized, sending IP");
        send_ipaddr(net_get_ip());

        // Start pinging the websocket server
        xTimerStart(_ctx.ping_timer, portMAX_DELAY);    
        return STATUS_OK;
    }

    return -STATUS_INVALID;
}

void client_handle_inbound_msg(cJSON *msg_in)
{
    // Parse command type
    msg_type_t msg_type = MSG_INVALID;
    if (cJSON_HasObjectItem(msg_in, "authorised"))
    { 
        msg_type = MSG_AUTHORISED; 
    }
    else
    {
        cJSON *msg_type_json = cJSON_GetObjectItem(msg_in, "command");
        if (msg_type_json)
        {
            msg_type = str_to_msgtype(msg_type_json->valuestring);
        }
    }

    // Send command to handlers
    for (int i=0; i<CLIENT_CMD_HANDLER_MAX; i++)
    {
        if (_ctx.handlers[i] != NULL)
        {
            if (_ctx.handlers[i](msg_type, msg_in) == STATUS_OK)
            {
                break;
            }
        }
    }
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