#include "config.h"
#include "device_config.h"

static const config_t config = {
    .device_type = CONFIG_DEVICE_TYPE,
    .portal = {
        .ws_url = CONFIG_PORTAL_WS_URL,
        .api_secret = CONFIG_PORTAL_API_SECRET,
    },
    .net = {
        .wifi_ssid = CONFIG_NET_WIFI_SSID,
        .wifi_pass = CONFIG_NET_WIFI_PASS,
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
        .wifi_power = CONFIG_DEBUG_WIFI_TX_POWER,
        .wifi_country_code = CONFIG_DEBUG_WIFI_COUNTRY_CODE,
        .cron_period = CONFIG_DEBUG_CRON_PERIOD,
    },
};

const config_t * config_get(void)
{
    return (const config_t *) &config;
}