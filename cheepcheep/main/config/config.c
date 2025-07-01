#include "config.h"
#include "device_config.h"
#include "log.h"
#include "nvstate.h"
#include "console.h"
#include "bsp.h"

// Set configs that haven't been set 
#ifndef CONFIG_DFU_URL
#define CONFIG_DFU_URL ""
#endif /*CONFIG_DFU_URL*/

#ifndef CONFIG_PORTAL_API_SECRET
#define CONFIG_PORTAL_API_SECRET ""
#endif /*CONFIG_PORTAL_API_SECRET*/

#ifndef CONFIG_PORTAL_WS_URL
#define CONFIG_PORTAL_WS_URL ""
#endif /*CONFIG_PORTAL_WS_URL*/

#ifndef CONFIG_NET_WIFI_SSID
#define CONFIG_NET_WIFI_SSID ""
#endif /*CONFIG_NET_WIFI_SSID*/

#ifndef CONFIG_NET_WIFI_PASS
#define CONFIG_NET_WIFI_PASS ""
#endif /*CONFIG_NET_WIFI_PASS*/

#ifndef CONFIG_INTLCK_USER
#define CONFIG_INTLCK_USER ""
#endif /*CONFIG_INTLCK_USER*/

#ifndef CONFIG_INTLCK_PASS
#define CONFIG_INTLCK_PASS ""
#endif /*CONFIG_INTLCK_PASS*/

int _set_defaults(int argc, char **argv);
int _set_wifi_ssid(int argc, char **argv);
int _set_wifi_pass(int argc, char **argv);
int _set_api_secret(int argc, char **argv);
int _set_api_url(int argc, char **argv);

static bool _init = false;
static config_t _config;

static const config_t _defaults = {
    .device_type = CONFIG_DEVICE_TYPE,
    .client = {
        .portal = {
            .ws_url = CONFIG_PORTAL_WS_URL,
            .api_secret = CONFIG_PORTAL_API_SECRET,
        },
        .net = {
            .wifi_ssid = CONFIG_NET_WIFI_SSID,
            .wifi_pass = CONFIG_NET_WIFI_PASS,
            .wifi_country_code = CONFIG_NET_WIFI_COUNTRY_CODE,
            .wifi_power = CONFIG_NET_WIFI_TX_POWER,
        },
        .dfu = {
            .url = CONFIG_DFU_URL,
            .skip_cn_check = CONFIG_DFU_SKIP_CN_CHECK,
            .skip_version_check = CONFIG_DFU_SKIP_VERSION_CHECK,
        },
    },
    .general = {
        .lock_reversed = CONFIG_GEN_LOCK_REVERSED,
        .reader_led_reversed = CONFIG_GEN_READER_LED_REV,
        .relay_reversed = CONFIG_GEN_RELAY_REV,
        .door_sensor_reversed = CONFIG_GEN_DOOR_SENSOR_REV,
        .door_sensor_enabled = CONFIG_GEN_DOOR_SENSOR_EN,
        .door_sensor_timeout = CONFIG_GEN_DOOR_SENSOR_TIMEOUT,
        .door_open_alarm_timeout = CONFIG_GEN_DOOR_OPEN_ALARM_TIMEOUT,
        .out_1_reversed = CONFIG_GEN_OUT1_REV,
        .in_1_reversed = CONFIG_GEN_IN1_REV,
        .aux_1_reversed = CONFIG_GEN_AUX1_REV,
        .aux_2_reversed = CONFIG_GEN_AUX2_REV,
        .fixed_unlock_delay = CONFIG_GEN_FIXED_UNLOCK_DELAY,
        .rgb_led_count = CONFIG_GEN_RGB_LED_COUNT,
        .wiegand_enabled = CONFIG_GEN_WIEGAND_ENABLED,
    },
    .buzzer = {
        .enabled = CONFIG_BUZZER_EN,
        .reversed = CONFIG_BUZZER_REV,
        .buzz_on_swipe = CONFIG_BUZZER_BUZZ_ON_SWIPE,
        .action_delay = CONFIG_BUZZER_ACTION_DELAY,
    },
    .interlock = {
        .tasmota_host = CONFIG_INTLCK_HOST,
        .tasmota_user = CONFIG_INTLCK_USER,
        .tasmota_pass = CONFIG_INTLCK_PASS,
    },
    .vending = {
        .price = CONFIG_VEND_PRICE,
        .mode = CONFIG_VEND_MODE,
        .toggle_time = CONFIG_VEND_TOGGLE_TIME,
    },
    .lcd = {
        .enable = CONFIG_LCD_EN,
        .address = CONFIG_LCD_ADDRESS,
        .cols = CONFIG_LCD_COLS,
        .rows = CONFIG_LCD_ROWS,
    },
    .pins = {
        .aux_1 = CONFIG_PINS_AUX1,
        .aux_2 = CONFIG_PINS_AUX2,
        .rgb_led = CONFIG_PINS_RGB_LED,
        .status_led = CONFIG_PINS_STATUS_LED,
        .reader_led = CONFIG_PINS_READER_LED,
        .reader_buzzer = CONFIG_PINS_READER_BUZZER,
        .relay = CONFIG_PINS_RELAY,
        .lock = CONFIG_PINS_LOCK,
        .door_sensor = CONFIG_PINS_DOOR_SENSOR,
        .out_1 = CONFIG_PINS_OUT1,
        .in_1 = CONFIG_PINS_IN1,
        .sda = CONFIG_PINS_SDA,
        .scl = CONFIG_PINS_SCL,
        .uart_rx = CONFIG_PINS_UART_RX,
        .uart_tx = CONFIG_PINS_UART_TX,
        .wiegand_zero = CONFIG_PINS_WIEGAND_ZERO,
        .wiegand_one = CONFIG_PINS_WIEGAND_ONE,
    },
    .dev = {
        .enable_wdt = CONFIG_DEV_ENABLE_WDT,
        .catch_all_exceptions = CONFIG_DEV_CATCH_EXCEPTIONS,
        .log_level = CONFIG_DEV_LOG_LEVEL,
        .enable_webrepl = CONFIG_DEV_ENABLE_WEBREPL,
    },
    .debug = {
        .enable_backup_server = CONFIG_DEBUG_ENABLE_BACKUP_SERVER,
        .uid_32bit_mode = CONFIG_DEBUG_UID_32BIT_MODE,
        .cron_period = CONFIG_DEBUG_CRON_PERIOD,
    },
};

status_t config_init(void)
{
    console_register("set_defaults", "set default config", NULL, _set_defaults);
    console_register("set_wifi_ssid", "set wifi network", NULL, _set_wifi_ssid);
    console_register("set_wifi_pass", "set wifi password", NULL, _set_wifi_pass);
    console_register("set_api_secret", "set api secret", NULL, _set_api_secret);
    console_register("set_api_url", "set api url", NULL, _set_api_url);

    INFO("Fetching configuration");
    status_t status = nvstate_config(&_config);
    if (status != STATUS_OK)
    {
        WARN("No config stored, saving defaults");
        status = nvstate_config_set(&_defaults);
        status |= nvstate_config(&_config);  
    }
    return status;
}

const config_t * config_get(void)
{
    status_t status;
    if (!_init)
    {
        if (config_init() != STATUS_OK)
        {
            return NULL;
        }
        _init = true;
    }

    return (const config_t *) &_config;
}

int _set_defaults(int arg, char **argv)
{
    printf("Setting default config. Reboot for defaults tot ake effect.\n");
    nvstate_config_set(&_defaults);
    nvstate_config(&_config);
    return 0;
}

int _set_wifi_ssid(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting Wifi SSID\n");
        strcpy(_config.client.net.wifi_ssid, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_wifi_pass(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting Wifi password\n");
        strcpy(_config.client.net.wifi_pass, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_api_secret(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting API secret\n");
        strcpy(_config.client.portal.api_secret, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_api_url(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting API URL\n");
        strcpy(_config.client.portal.ws_url, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}