#include "device_door.h"
#include "log.h"
#include "bsp.h"
#include "tags.h"
#include "nvstate.h"
#include "signal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    config_general_t config;
    bool prev_door_open_state;
    int64_t time_opened;
    uint32_t last_card_id;
} door_ctx_t;

static status_t door_init(const config_t *config);
static void door_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

void door_task(void *params);
static void lock_door(void);
static void unlock_door(void);

device_t door = {
    .init = door_init,
    .swipe_cb = door_handle_swipe, 
};

door_ctx_t _ctx = {
    .prev_door_open_state = false,
    .time_opened = 0,
    .last_card_id = 0,
};

static status_t door_init(const config_t *config)
{
    memcpy(&_ctx.config, &config->general, sizeof(config_general_t));
    
    signal_init();
    
    xTaskCreate(door_task, "Door_Task", 2048, (void *)&_ctx, 1, NULL);
    return STATUS_OK;
}

static void door_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{
    INFO("New card:\n");
    INFO("    facility: %d\n", card->facility);
    INFO("    user id:  %d\n", card->user_id);
    INFO("    raw:      %ld\n", card->raw);

    if (true /*buzz_on_swipe*/)
    {
        // hardware.buzz_card_read()
        gpio_out_set(OUTPUT_READER_BUZZER, true);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_out_set(OUTPUT_READER_BUZZER, false);
    }

    if (tags_find(card->raw) == STATUS_OK)
    {
        if (nvstate_locked_out())
        {
            // TODO: Send to websocket when it exists
            //log_door_swipe(card, locked_out=True)
            signal_alert();
        }
        else
        {
            // TODO: Send to websocket when it exists
            //log_door_swipe(card)
            unlock_door();
        }
    }
    else
    {
        // TODO: Send to websocket when it exists
        //log_door_swipe(card, rejected=True)
        signal_alert();
    }

    _ctx.last_card_id = card->raw;
}

void door_task(void *params)
{
    door_ctx_t *ctx = (door_ctx_t *) params;
    while(true)
    {
        bool cur_door_state = gpio_in_get(INPUT_DOOR_SENSOR);
        if (ctx->prev_door_open_state != cur_door_state) 
        {
            ctx->prev_door_open_state = cur_door_state;
            INFO("Door sensor state changed to %u", cur_door_state);
            if (cur_door_state)
            {
                ctx->time_opened = uptime();
            }
            else
            {
                ctx->time_opened = 0;
            }

            // If door sensor is enabled and the door is currently open
            if (ctx->config.door_sensor_enabled && ctx->prev_door_open_state)
            {
                // Check the timeout condition
                if (ctx->prev_door_open_state && ((uptime() - ctx->time_opened) >= (ctx->config.door_sensor_timeout * 1000)))
                {
                    INFO("Door sensor timeout! Locking again.");
                    lock_door();
                }
                else if (!ctx->prev_door_open_state)
                {
                    INFO("Door opened while waiting, locking door in 0.5s");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    lock_door();
                }
            }

            if (ctx->config.door_open_alarm_timeout != 0 && ctx->prev_door_open_state)
            {
                if (uptime() - ctx->time_opened >= (ctx->config.door_open_alarm_timeout * 1000))
                {
                    WARN("Door left open alarm!");
                    gpio_out_set(OUTPUT_READER_BUZZER, true);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void lock_door(void)
{
    gpio_out_set(OUTPUT_LOCK, true);
    gpio_out_set(OUTPUT_RELAY, false);
    WARN("Locked!");
}

static void unlock_door(void)
{
    gpio_out_set(OUTPUT_LOCK, false);
    gpio_out_set(OUTPUT_RELAY, true);
    WARN("Unlocked!");
    signal_ok();

    if (_ctx.config.door_sensor_enabled)
    {
        _ctx.time_opened = uptime();
    }
    else
    {
        vTaskDelay(pdMS_TO_TICKS(_ctx.config.fixed_unlock_delay * 1000));
        lock_door();
    }
}