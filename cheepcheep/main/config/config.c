#include "config.h"
#include "config_defaults.h"
#include "log.h"
#include "nvstate.h"
#include "console.h"
#include "bsp.h"

static bool _init = false;
static config_t _config;

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
int _set_buzz_en(int argc, char **argv);
int _set_buzz_rev(int argc, char **argv);
int _set_buzz_on_swipe(int argc, char **argv);
int _set_buzz_action_delay(int argc, char **argv);
int _set_log_level(int argc, char **argv);
int _set_lock_rev(int argc, char **argv);
int _set_reader_led_rev(int argc, char **argv);
int _set_relay_rev(int argc, char **argv);
int _set_door_sensor_rev(int argc, char **argv);
int _set_door_sensor_en(int argc, char **argv);
int _set_door_sensor_timeout(int argc, char **argv);
int _set_door_open_alarm_timeout(int argc, char **argv);
int _set_out_1_rev(int argc, char **argv);
int _set_in_1_rev(int argc, char **argv);
int _set_aux_1_rev(int argc, char **argv);
int _set_aux_2_rev(int argc, char **argv);
int _set_fixed_unlock_delay(int argc, char **argv);
int _set_rgb_led_count(int argc, char **argv);
int _set_wiegand_en(int argc, char **argv);
int _set_32bit_mode(int argc, char **argv);

status_t config_init(void)
{
    // Meta
    console_register("factory_reset", "set default config", NULL, _set_defaults);
    
    // device_type
    console_register("device_type", "set device type", NULL, _set_device_type);
    
    // client.portal
    console_register("api_secret", "set api secret", NULL, _set_api_secret);
    console_register("api_url", "set api url", NULL, _set_api_url);

    // client.net
    console_register("wifi_ssid", "set wifi network", NULL, _set_wifi_ssid);
    console_register("wifi_pass", "set wifi password", NULL, _set_wifi_pass);
    console_register("txpow", "set wifi tx power", NULL, _set_txpow);
    console_register("country", "set wifi country code", NULL, _set_wifi_country);

    // client.dfu
    console_register("dfu_url", "set dfu url", NULL, _set_dfu_url);
    console_register("dfu_skip_cn", "skip common name verification for DFU server", NULL, _set_dfu_skipcncheck);
    console_register("dfu_skip_ver", "skip version check during DFU", NULL, _set_dfu_skipvercheck);

    // general
    console_register("lock_rev", "reverse lock polarity", NULL, _set_lock_rev);
    console_register("reader_led_rev", "reverse reader led polarity", NULL, _set_reader_led_rev);
    console_register("relay_rev", "reverse rrelay polarity", NULL, _set_relay_rev);
    console_register("door_sensor_rev", "reverse door sensor polarity", NULL, _set_door_sensor_rev);
    console_register("door_sensor_en", "enable door sensor", NULL, _set_door_sensor_en);
    console_register("door_sensor_timeout", "set door sensor timeout", NULL, _set_door_sensor_timeout);
    console_register("door_open_alarm_timeout", "set door open alarm timeout", NULL, _set_door_open_alarm_timeout);
    console_register("out1_rev", "reverse out1 polarity", NULL, _set_out_1_rev);
    console_register("in1_rev", "reverse in1 polarity", NULL, _set_in_1_rev);
    console_register("aux1_rev", "reverse aux1 polarity", NULL, _set_aux_1_rev);
    console_register("aux2_rev", "reverse aux2 polarity", NULL, _set_aux_2_rev);
    console_register("fixed_unlock_time", "set fixed unlock time", NULL, _set_fixed_unlock_delay);
    console_register("rgb_leds", "set number of rgb leds", NULL, _set_rgb_led_count);
    console_register("wiegand", "Enable/disable wiegand", NULL, _set_wiegand_en);
    console_register("32bit_mode", "set 32bit mode", NULL, _set_32bit_mode);

    // buzzer
    console_register("buzz_enable", "enable/disable the buzzer", NULL, _set_buzz_en);
    console_register("buzz_rev", "reverse buzzer polarity", NULL, _set_buzz_rev);
    console_register("buzz_on_swipe", "Enable buzzing on swipe", NULL, _set_buzz_on_swipe);
    console_register("buzz_action_delay", "Set buzzer action delay", NULL, _set_buzz_action_delay);

    // interlock
    console_register("ilock_url", "set interlock tasmota host", NULL, _set_ilock_url);
    console_register("ilock_user", "set interlock tasmota username", NULL, _set_ilock_user);
    console_register("ilock_pass", "set interlock tasmota password", NULL, _set_ilock_pass);

    // vending
    // TODO: unimpl right now

    // lcd
    // TODO: unimpl right now

    // pins
    // TODO: do last

    // dev
    console_register("log", "set log level", NULL, _set_log_level);
    
    INFO("Fetching configuration");
    status_t status = nvstate_config(&_config);
    if (status != STATUS_OK)
    {
        WARN("No config stored, saving defaults");
        status = nvstate_config_set((config_t *) &_defaults);
        status |= nvstate_config(&_config);  
    }
    return status;
}

const config_t * config_get(void)
{
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

int _set_buzz_en(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting buzzer enable\n");
        _config.buzzer.enabled = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;    
}

int _set_buzz_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting buzzer reverse\n");
        _config.buzzer.reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;    
}

int _set_buzz_on_swipe(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting buzzer on swipe\n");
        _config.buzzer.buzz_on_swipe = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;  
}

int _set_buzz_action_delay(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting buzzer action delay\n");
        _config.buzzer.action_delay = (int) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;  
}

int _set_log_level(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting log level\n");
        if (strcmp("error", argv[1]) == 0)
        {
            _config.dev.log_level = LOG_ERROR;
        }
        else if (strcmp("warning", argv[1]) == 0)
        {
            _config.dev.log_level = LOG_WARNING;        
        }
        else if (strcmp("info", argv[1]) == 0)
        {
            _config.dev.log_level = LOG_INFO;
        }
        else if (strcmp("debug", argv[1]) == 0)
        {
            _config.dev.log_level = LOG_DEBUG;
        }
        else if (strcmp("verbose", argv[1]) == 0)
        {
            _config.dev.log_level = LOG_VERBOSE;
        }
        else
        {
            printf("Invalid log string - choose error, warning, info, debug, or verbose\n");
            return 0;
        }
        nvstate_config_set(&_config);
    }
    return 0;        
}

int _set_lock_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting lock rev\n");
        _config.general.lock_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;  
}

int _set_reader_led_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting reader led rev\n");
        _config.general.reader_led_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_relay_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting relay rev\n");
        _config.general.relay_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_door_sensor_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting door sensor rev\n");
        _config.general.door_sensor_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_door_sensor_en(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting door sensor enable\n");
        _config.general.door_sensor_enabled = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_door_sensor_timeout(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting door sensor timeout\n");
        _config.general.door_sensor_timeout = (int) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_door_open_alarm_timeout(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting door open alarm timeout\n");
        _config.general.door_open_alarm_timeout = (int) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_out_1_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting out 1 rev\n");
        _config.general.out_1_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_in_1_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting in 1 rev\n");
        _config.general.in_1_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_aux_1_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting aux 1 rev\n");
        _config.general.aux_1_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_aux_2_rev(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting aux 2 rev\n");
        _config.general.aux_2_reversed = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_fixed_unlock_delay(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting fixed unlock delay\n");
        _config.general.fixed_unlock_delay = (int) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_rgb_led_count(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting rgb led count\n");
        _config.general.rgb_led_count = (int) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0;
}

int _set_wiegand_en(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting wiegand enable\n");
        _config.general.wiegand_enabled = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}

int _set_32bit_mode(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("Setting 32bit card mode\n");
        _config.general.uid_32bit_mode = (bool) atoi(argv[1]);
        nvstate_config_set(&_config);
    }
    return 0; 
}