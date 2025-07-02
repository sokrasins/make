#include "config.h"
#include "device_config.h"
#include "config_defaults.h"
#include "log.h"
#include "nvstate.h"
#include "console.h"
#include "bsp.h"



#ifndef CONFIG_INTLCK_USER
#define CONFIG_INTLCK_USER ""
#endif /*CONFIG_INTLCK_USER*/

#ifndef CONFIG_INTLCK_PASS
#define CONFIG_INTLCK_PASS ""
#endif /*CONFIG_INTLCK_PASS*/

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
        .uid_32bit_mode = CONFIG_GEN_UID_32BIT_MODE,
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
        .log_level = CONFIG_DEV_LOG_LEVEL,
    },
};

int _set_defaults(int argc, char **argv);
int _set_wifi_ssid(int argc, char **argv);
int _set_wifi_pass(int argc, char **argv);
int _set_api_secret(int argc, char **argv);
int _set_api_url(int argc, char **argv);
int _set_dfu_url(int argc, char **argv);
int _set_ilock_url(int argc, char **argv);
int _set_ilock_user(int argc, char **argv);
int _set_ilock_pass(int argc, char **argv);
int _set_device_type(int argc, char **argv);
int _set_txpow(int argc, char **argv);
int _set_wifi_country(int argc, char **argv);
int _set_dfu_skipcncheck(int argc, char **argv);
int _set_dfu_skipvercheck(int argc, char **argv);

status_t config_init(void)
{
    // Meta
    console_register("factory_reset", "set default config", NULL, _set_defaults);
    
    // device_type
    console_register("device_type", "set device type", NULL, _set_device_type);
    
    // client.net
    console_register("wifi_ssid", "set wifi network", NULL, _set_wifi_ssid);
    console_register("wifi_pass", "set wifi password", NULL, _set_wifi_pass);
    console_register("txpow", "set wifi tx power", NULL, _set_txpow);
    console_register("country", "set wifi country code", NULL, _set_wifi_country);

    // client.portal
    console_register("api_secret", "set api secret", NULL, _set_api_secret);
    console_register("api_url", "set api url", NULL, _set_api_url);
    
    // client.dfu
    console_register("dfu_url", "set dfu url", NULL, _set_dfu_url);
    console_register("dfu_skip_cn", "skip common name verification for DFU server", NULL, _set_dfu_skipcncheck);
    console_register("dfu_skip_ver", "skip version check during DFU", NULL, _set_dfu_skipvercheck);

    // general

    // buzzer

    // interlock
    console_register("ilock_url", "set interlock tasmota host", NULL, _set_ilock_url);
    console_register("ilock_user", "set interlock tasmota username", NULL, _set_ilock_user);
    console_register("ilock_pass", "set interlock tasmota password", NULL, _set_ilock_pass);

    // lcd

    // pins

    // dev

    // debug

    
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

int _set_dfu_url(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting DFU URL\n");
        strcpy(_config.client.dfu.url, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_ilock_user(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting interlock tasmota username\n");
        strcpy(_config.interlock.tasmota_user, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_ilock_pass(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting interlock tasmota password\n");
        strcpy(_config.interlock.tasmota_pass, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_device_type(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting device type\n");
        if (strcmp("door", argv[1]) == 0)
        {
            _config.device_type = DEVICE_DOOR;
        }
        else if (strcmp("interlock", argv[1]) == 0)
        {
            _config.device_type = DEVICE_INTERLOCK;
        }
        else if (strcmp("vending", argv[1]) == 0)
        {
            _config.device_type = DEVICE_VENDING;
        }
        else
        {
            printf("Invalid device string - choose door, interlock, or vending\n");
            return 0;
        }
        nvstate_config_set(&_config);
    }
    return 0;    
}

int _set_txpow(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("TX power setting not implemented\n");
        //int pow = atoi(argv[1]);
        //_config.client.net.wifi_power = pow;
        //nvstate_config_set(&_config);
    }
    return 0;
}

int _set_wifi_country(int argc, char **argv)
{
    if (argc == 2)
    {
        if (strlen(argv[1]) == 2)
        {
            printf("Setting wifi country code\n");
            strcpy(_config.client.net.wifi_country_code, argv[1]);
            nvstate_config_set(&_config);
        }
        else
        {
            printf("Country code must be 2 characters (e.g. \"US\")");
        }
    }
    return 0;
}

int _set_ilock_url(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting interlock tasmota URL\n");
        strcpy(_config.interlock.tasmota_host, argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_dfu_skipcncheck(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting dfu common name check skip\n");
        _config.client.dfu.skip_cn_check = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;  
}

int _set_dfu_skipvercheck(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting dfu version check skip\n");
        _config.client.dfu.skip_version_check = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;    
}
