#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

#include "config_types.h"
#include "device_creds.h"

#define CONFIG_DEVICE_TYPE                  DEVICE_DOOR       

#define CONFIG_NET_WIFI_TX_POWER            WIFI_POW_MAX
#define CONFIG_NET_WIFI_COUNTRY_CODE        "US"

#define CONFIG_GEN_LOCK_REVERSED            false
#define CONFIG_GEN_READER_LED_REV           true
#define CONFIG_GEN_RELAY_REV                false
#define CONFIG_GEN_DOOR_SENSOR_REV          true
#define CONFIG_GEN_DOOR_SENSOR_EN           false
#define CONFIG_GEN_DOOR_SENSOR_TIMEOUT      5
#define CONFIG_GEN_DOOR_OPEN_ALARM_TIMEOUT  0
#define CONFIG_GEN_OUT1_REV                 false
#define CONFIG_GEN_IN1_REV                  true
#define CONFIG_GEN_AUX1_REV                 false
#define CONFIG_GEN_AUX2_REV                 false
#define CONFIG_GEN_FIXED_UNLOCK_DELAY       5
#define CONFIG_GEN_RGB_LED_COUNT            1
#define CONFIG_GEN_WIEGAND_ENABLED          true

#define CONFIG_BUZZER_EN                    true
#define CONFIG_BUZZER_REV                   true
#define CONFIG_BUZZER_BUZZ_ON_SWIPE         true
#define CONFIG_BUZZER_ACTION_DELAY          2

#define CONFIG_INTLCK_HOST                  ""

#define CONFIG_VEND_PRICE                   250
#define CONFIG_VEND_MODE                    VENDING_HOLD
#define CONFIG_VEND_TOGGLE_TIME             1

#define CONFIG_LCD_EN                       true
#define CONFIG_LCD_ADDRESS                  0x27
#define CONFIG_LCD_COLS                     16
#define CONFIG_LCD_ROWS                     2

#define CONFIG_PINS_AUX1                    2
#define CONFIG_PINS_AUX2                    1
#define CONFIG_PINS_RGB_LED                 37
#define CONFIG_PINS_STATUS_LED              38
#define CONFIG_PINS_READER_LED              5
#define CONFIG_PINS_READER_BUZZER           4
#define CONFIG_PINS_RELAY                   36
#define CONFIG_PINS_LOCK                    14
#define CONFIG_PINS_DOOR_SENSOR             12
#define CONFIG_PINS_OUT1                    35
#define CONFIG_PINS_IN1                     13
#define CONFIG_PINS_SDA                     48
#define CONFIG_PINS_SCL                     47
#define CONFIG_PINS_UART_TX                 15
#define CONFIG_PINS_UART_RX                 16
#define CONFIG_PINS_WIEGAND_ZERO            7
#define CONFIG_PINS_WIEGAND_ONE             6

#define CONFIG_DEV_ENABLE_WDT               false
#define CONFIG_DEV_CATCH_EXCEPTIONS         false
#define CONFIG_DEV_LOG_LEVEL                LOG_DEBUG
#define CONFIG_DEV_ENABLE_WEBREPL           false

#define CONFIG_DEBUG_ENABLE_BACKUP_SERVER   false
#define CONFIG_DEBUG_UID_32BIT_MODE         false
#define CONFIG_DEBUG_CRON_PERIOD            10000

#endif /*DEVICE_CONFIG_H_*/