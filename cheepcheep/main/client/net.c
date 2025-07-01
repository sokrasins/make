#include "net.h"
#include "log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"

#define WIFI_RETRIES         5U
#define NET_EVT_HANDLERS_NUM 10

#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_DISCONNECTED_BIT   BIT1

typedef struct {
    net_evt_t evt;
    void *ctx;
    net_evt_cb_t cb;
} net_evt_handler_t;

typedef struct {
    net_evt_handler_t handlers[NET_EVT_HANDLERS_NUM];
    const config_network_t *config;
    EventGroupHandle_t wifi_event_group;
    int retry_num;
    esp_ip4_addr_t ip;
} net_ctx_t;

static net_ctx_t _ctx = {
    .retry_num = 0,
};

static void net_task(void *params);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

status_t net_init(const config_network_t *config)
{
    if (strlen(config->wifi_ssid) == 0)
    {
        ERROR("No Wifi SSID provided, please set.");
        return -STATUS_BAD_CONFIG;
    }
    if (strlen(config->wifi_pass) == 0)
    {
        ERROR("No Wifi password provided, please set.");
        return -STATUS_BAD_CONFIG;
    }

    _ctx.config = config;
    _ctx.wifi_event_group = xEventGroupCreate();

    for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
    {
        _ctx.handlers[i].cb = NULL;
    }

    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("wifi_init", ESP_LOG_WARN);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_WARN);

    // Set up the itf
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    
    // Start wifi
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    return STATUS_OK;
}

status_t net_start(void)
{
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // Run net task, ready to handle network state changes
    xTaskCreate(net_task, "Net_Task", 4096, (void *)&_ctx, 3, NULL);
    
    // Register our event handler
    esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        NULL,
        &instance_any_id
    );
    esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip
    );

    // Set up wifi config
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
        },
    };
    memcpy(wifi_config.sta.ssid, _ctx.config->wifi_ssid, strlen(_ctx.config->wifi_ssid));
    memcpy(wifi_config.sta.password, _ctx.config->wifi_pass, strlen(_ctx.config->wifi_pass));

    // Set wifi params from config
    esp_wifi_set_max_tx_power(_ctx.config->wifi_power);
    esp_wifi_set_country_code(_ctx.config->wifi_country_code, true);
    
    // Configure wifi
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    esp_wifi_set_ps(WIFI_PS_NONE);
    
    // Start wifi
    esp_wifi_start();

    return STATUS_OK; 
}

net_evt_handle_t net_evt_cb_register(net_evt_t evt, void *ctx, net_evt_cb_t cb)
{
    for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
    {
        net_evt_handler_t *handler = &_ctx.handlers[i];
        if (handler->cb == NULL)
        {
            handler->evt = evt;
            handler->ctx = ctx;
            handler->cb = cb;
            return (net_evt_handle_t) handler;
        }
    }
    return NULL;
}

void net_evt_cb_deregister(net_evt_handle_t handle)
{
    net_evt_handler_t *handler = (net_evt_handler_t *) handle;
    handler->cb = NULL;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        INFO("STA START");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (_ctx.retry_num < WIFI_RETRIES) 
        {
            esp_wifi_connect();
            _ctx.retry_num++;
            INFO("retry to connect to the AP");
        } 
        else 
        {
            INFO("STA_DISCONNECTED");
            _ctx.retry_num = 0;
            xEventGroupSetBits(_ctx.wifi_event_group, WIFI_DISCONNECTED_BIT);
        }
        INFO("connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        INFO("IP address assigned: " IPSTR, IP2STR(&event->ip_info.ip));
        memcpy(&_ctx.ip, &event->ip_info.ip, sizeof(_ctx.ip));
        _ctx.retry_num = 0;
        xEventGroupSetBits(_ctx.wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void net_task(void *params)
{
    net_ctx_t *ctx = (net_ctx_t *) params;
    while (1)
    {
        // Waiting until either the connection is established (WIFI_CONNECTED_BIT) 
        // or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). 
        // The bits are set by event_handler() (see above)
        EventBits_t bits = xEventGroupWaitBits(
            ctx->wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if(bits & WIFI_CONNECTED_BIT)
        {
            INFO("connected to ap SSID: %s", ctx->config->wifi_ssid);

            for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
            {
                net_evt_handler_t *handler = &ctx->handlers[i];
                if (handler->evt == NET_EVT_CONNECT && handler->cb != NULL)
                {
                    handler->cb(NET_EVT_CONNECT, handler->ctx);
                }
            }
        }

        if (bits & WIFI_DISCONNECTED_BIT)
        {
            ERROR("Failed to connect to SSID: %s", ctx->config->wifi_ssid);

            for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
            {
                net_evt_handler_t *handler = &ctx->handlers[i];
                if (handler->evt == NET_EVT_DISCONNECT && handler->cb != NULL)
                {
                    handler->cb(NET_EVT_DISCONNECT, handler->ctx);
                }
            }

            // Try to reconnect
            esp_wifi_connect();
        }
    }
}

void net_get_mac(uint8_t *mac)
{
    esp_wifi_get_mac(WIFI_IF_STA, mac);
}

uint32_t net_get_ip(void)
{
    return _ctx.ip.addr;
}