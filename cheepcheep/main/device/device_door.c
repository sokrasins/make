#include "device_door.h"
#include "log.h"
#include "bsp.h"
#include "tags.h"
#include "nvstate.h"
#include "signal.h"
#include "wiegand.h"
#include "client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    const config_general_t *config;
    bool prev_door_open_state;
    bool unlock_door;
    int64_t time_opened;
    int64_t time_unlocked;
    uint32_t last_card_id; // TODO: debounce reads
    wieg_evt_handle_t evt_handle;
} door_ctx_t;

// API
static status_t door_init(const config_t *config);
static void door_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

// Private
void door_task(void *params);
static void lock_door(void);
static void unlock_door(void);
static status_t client_cmd_handler(msg_t *msg);

device_t door = {
    .init = door_init,
};

static door_ctx_t _ctx = {
    .prev_door_open_state = false,
    .time_opened = 0,
    .time_unlocked = 0,
    .last_card_id = 0,
};

static status_t door_init(const config_t *config)
{
    signal_init(&config->buzzer);

    _ctx.config = &config->general;
    _ctx.evt_handle = wieg_evt_handler_reg(WIEG_EVT_NEWCARD, door_handle_swipe, (void *)&_ctx);

    // Register cb for server requests
    client_handler_register(client_cmd_handler);

    xTaskCreate(door_task, "Door_Task", 2048, (void *)&_ctx, 1, NULL);
    return STATUS_OK;
}

static void door_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{
    door_ctx_t *door_ctx = (door_ctx_t *) ctx;

    INFO("New card");
    INFO("    facility: 0x%hx", card->facility);
    INFO("    user id:  0x%hx", card->user_id);
    INFO("    raw:      %d", card->raw);

    signal_cardread();

    // Consider doin this in a task
    if (tags_find(card->raw) == STATUS_OK)
    {
        if (nvstate_locked_out())
        {
            msg_t msg = {
                .type = MSG_ACCESS_LOCKED_OUT,
                .access_lockout.card_id = card->raw,
            };
            client_send_msg(&msg);
            signal_alert();
        }
        else
        {
            msg_t msg = {
                .type = MSG_ACCESS_GRANTED,
                .access_granted.card_id = card->raw,
            };
            client_send_msg(&msg);
            door_ctx->unlock_door = true;
        }
    }
    else
    {
        msg_t msg = {
            .type = MSG_ACCESS_DENIED,
            .access_denied.card_id = card->raw,
        };
        client_send_msg(&msg);
        signal_alert();
    }

    _ctx.last_card_id = card->raw;
}

void door_task(void *params)
{
    door_ctx_t *ctx = (door_ctx_t *) params;
    while(true)
    {
        // If requested, unlock the door
        if (ctx->unlock_door)
        {
            unlock_door();
            ctx->unlock_door = false;
        }

        // If door sensor is not enabled, close door after fixed time
        if(!ctx->config->door_sensor_enabled && ctx->time_unlocked != 0)
        {
            if ((uptime() - ctx->time_unlocked) >= (_ctx.config->fixed_unlock_delay * 1000))
            {
                lock_door();
            }
        }

        // If door sensor is enabled, monitor the door state
        if (ctx->config->door_sensor_enabled)
        {
            // Get the current door state
            int rc = gpio_in_get(INPUT_DOOR_SENSOR);
            if (rc < 0)
            {
                continue;
            }
            bool cur_door_state = (bool) rc;
            
            // Check if the door has opened or closed
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
                    // Buzzer needs to stop if the open door alarm is on
                    gpio_out_set(OUTPUT_READER_BUZZER, false);
                    ctx->time_opened = 0;
                }
            }

            // If the door was unlocked, check to make sure the door is actually opened
            if (ctx->time_unlocked != 0)
            {
                if ((uptime() - ctx->time_unlocked) >= (ctx->config->door_sensor_timeout * 1000))
                {
                    // If the door never opens, lock it again
                    INFO("Door sensor timeout! Locking again.");
                    lock_door();
                }
                else if (ctx->prev_door_open_state)
                {
                    // If the door is opened, then re-lock it
                    INFO("Door opened while waiting, locking door in 0.5s");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    lock_door();
                }
            }

            // If there's a timeout for the open door, check how long it's been opened
            if (ctx->config->door_open_alarm_timeout != 0 && ctx->time_opened != 0)
            {
                if (uptime() - ctx->time_opened >= (ctx->config->door_open_alarm_timeout * 1000))
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
    _ctx.time_unlocked = 0;
    WARN("Locked!");
}

static void unlock_door(void)
{
    gpio_out_set(OUTPUT_LOCK, false);
    gpio_out_set(OUTPUT_RELAY, true);
    WARN("Unlocked!");
    signal_ok();

    _ctx.time_unlocked = uptime();
}

static status_t client_cmd_handler(msg_t *msg)
{
    status_t status = -STATUS_UNAVAILABLE;

    if (msg->type == MSG_BUMP)
    {
        WARN("BUMP!");
        _ctx.unlock_door = true;
        status = STATUS_OK;
    }
    if (msg->type == MSG_UNLOCK)
    {
        WARN("UNLOCK!");
        gpio_out_set(OUTPUT_LOCK, false);
        status = STATUS_OK;
    }
    if (msg->type == MSG_LOCK)
    {
        WARN("LOCK!");
        gpio_out_set(OUTPUT_LOCK, true);
        status = STATUS_OK;
    }

    return status;
}