#include "net.h"
#include "log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h" //esp_init funtions esp_err_t 
#include "esp_wifi.h" //esp_wifi_init functions and wifi operations
#include "esp_event.h" //for wifi event
#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps

#define WIFI_RETRIES 5U

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define NET_EVT_HANDLERS_NUM 10

typedef struct {
    net_evt_t evt;
    void *ctx;
    net_evt_cb_t cb;
} net_evt_handler_t;

static status_t wifi_connection(void);
static void net_task(void *params);

net_evt_handler_t _handlers[NET_EVT_HANDLERS_NUM];
static const config_network_t *_config;
static EventGroupHandle_t wifi_event_group;
int retry_num = 0;
bool connected = false;
esp_ip4_addr_t _ip;

status_t net_init(const config_network_t *config)
{
    _config = config;
    wifi_event_group = xEventGroupCreate();

    for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
    {
        _handlers[i].cb = NULL;
    }

    return STATUS_OK;
}

status_t net_start(void)
{
    status_t status = wifi_connection();
    if (status == STATUS_OK)
    {
        xTaskCreate(net_task, "Net_Task", 4096, NULL, 3, NULL);
    }
    return status;    
}

net_evt_handle_t net_evt_cb_register(net_evt_t evt, void *ctx, net_evt_cb_t cb)
{
    for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
    {
        net_evt_handler_t *handler = &_handlers[i];
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

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        connected = false;
        if (retry_num < WIFI_RETRIES) 
        {
            esp_wifi_connect();
            retry_num++;
            INFO("retry to connect to the AP");
        } 
        else 
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
        INFO("connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        INFO("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;
        connected = true;
        memcpy(&_ip, &event->ip_info.ip, sizeof(_ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static status_t wifi_connection(void)
{
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // Set up the itf
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    
    // Start wifi
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    
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
    memcpy(wifi_config.sta.ssid, _config->wifi_ssid, strlen(_config->wifi_ssid));
    memcpy(wifi_config.sta.password, _config->wifi_pass, strlen(_config->wifi_pass));

    // Set wifi params from config
    esp_wifi_set_max_tx_power(_config->wifi_power);
    esp_wifi_set_country_code(_config->wifi_country_code, true);
    
    // Configure wifi
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    
    // Start wifi
    esp_wifi_start();

    return STATUS_OK;
}

static void net_task(void *params)
{
    while (1)
    {
        // Waiting until either the connection is established (WIFI_CONNECTED_BIT) 
        // or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). 
        // The bits are set by event_handler() (see above)
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if (bits & WIFI_CONNECTED_BIT) 
        {
            INFO("connected to ap SSID: %s", _config->wifi_ssid);

            for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
            {
                net_evt_handler_t *handler = &_handlers[i];
                if (handler->evt == NET_EVT_CONNECT && handler->cb != NULL)
                {
                    handler->cb(NET_EVT_CONNECT, handler->ctx);
                }
            }
        } 
        else if (bits & WIFI_FAIL_BIT) 
        {
            ERROR("Failed to connect to SSID: %s", _config->wifi_ssid);

            for (int i=0; i<NET_EVT_HANDLERS_NUM; i++)
            {
                net_evt_handler_t *handler = &_handlers[i];
                if (handler->evt == NET_EVT_DISCONNECT && handler->cb != NULL)
                {
                    handler->cb(NET_EVT_DISCONNECT, handler->ctx);
                }
            }
        } 
        else 
        {
            ERROR("UNEXPECTED EVENT");
        }
        if (!connected)
        {
            ERROR("No wifi connection");
            // TODO: Blink LED
            // TODO: Try to reconnect?
        }
    }
}

void net_get_mac(uint8_t *mac)
{
    esp_wifi_get_mac(WIFI_IF_STA, mac);
}

uint32_t net_get_ip(void)
{
    return _ip.addr;
}