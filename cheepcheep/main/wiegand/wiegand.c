#include "wiegand.h"
#include "wiegand_fmt.h"
#include "log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include <stddef.h>
#include <string.h>
#include <assert.h>

#define WIEG_MAX_HANDLERS 10U
#define WIEG_TIMEOUT      20U //ms

// Types
typedef enum {
    PARITY_EVEN,
    PARITY_ODD,
} parity_t;

typedef struct {
    void *ctx;
    wieg_evt_t event;
    wieg_evt_cb_t cb;
} handlers_t;

typedef struct {
    uint64_t bits;
    int ptr;
} wieg_data_t;

typedef struct {
    handlers_t handlers[WIEG_MAX_HANDLERS];
    wieg_data_t data;
    const wieg_fmt_t *fmt;
    QueueHandle_t pin_q;
} wieg_ctx_t;

// Vars
static wieg_ctx_t _ctx;

// Helpers
void wieg_task(void *params);

static bool wieg_is_card_valid(const wieg_fmt_t *fmt, uint32_t bits);
static void bits_to_card(const wieg_fmt_t *fmt, uint64_t bits, card_t *card);
static bool parity(parity_t parity, uint64_t num);
static void gpio_interrupt_handler(void *args);

status_t wieg_init(int d0, int d1, wieg_encoding_t encode)
{
    // Init ctx
    _ctx.data.bits = 0;
    _ctx.data.ptr = 0;
    _ctx.fmt = encode == WIEG_26_BIT ? &wieg_fmt_26bit : &wieg_fmt_34bit;

    for (int i=0; i<WIEG_MAX_HANDLERS; i++)
    {
        _ctx.handlers[i].cb = NULL;
    }

    // Set up gpio
    gpio_set_direction(d0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(d0, GPIO_FLOATING);
    gpio_set_intr_type(d0, GPIO_INTR_NEGEDGE);

    gpio_set_direction(d1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(d1, GPIO_FLOATING);
    gpio_set_intr_type(d1, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(d0, gpio_interrupt_handler, (void *)0);
    gpio_isr_handler_add(d1, gpio_interrupt_handler, (void *)1);

    // Set up queue for new bits and start task
    _ctx.pin_q = xQueueCreate(_ctx.fmt->total_bits, sizeof(int));
    xTaskCreate(wieg_task, "Wiegand_Task", 2048, &_ctx, 1, NULL);

    return STATUS_OK;
}

wieg_evt_handle_t wieg_evt_handler_reg(wieg_evt_t event, wieg_evt_cb_t cb, void *ctx)
{
    assert(cb);

    if (event == WIEG_EVT_NEWBIT)
    {
        return NULL;
    }

    for (int i=0; i<WIEG_MAX_HANDLERS; i++)
    {
        if (_ctx.handlers[i].cb == NULL)
        {
            _ctx.handlers[i].event = event;
            _ctx.handlers[i].cb = cb;
            _ctx.handlers[i].ctx = ctx;
            return (void *) &_ctx.handlers[i];
        }
    }
    return NULL;
}

status_t wieg_evt_handler_dereg(wieg_evt_handle_t handle)
{
    assert(handle);

    handlers_t *handler = (handlers_t *) handle;
    handler->cb = NULL;
    return STATUS_OK;
}

// Private

void wieg_task(void *params)
{
    wieg_ctx_t *ctx = (wieg_ctx_t *) params;

    while (1)
    {
        int bit;
        if (xQueueReceive(ctx->pin_q, &bit, pdMS_TO_TICKS(WIEG_TIMEOUT)))
        {
            // Record new bit to data
            ctx->data.bits |= bit << ctx->data.ptr++;

            // TODO: Handle new bit event

            // Check if we have complete card data yet
            if (ctx->data.ptr >= ctx->fmt->total_bits)
            {
                // Verify card data
                if (wieg_is_card_valid(ctx->fmt, ctx->data.bits))
                {
                    DEBUG("New card received");

                    // Fire NEWCARD events
                    for (int i=0; i<WIEG_MAX_HANDLERS; i++)
                    {
                        if(ctx->handlers[i].cb != NULL && ctx->handlers[i].event == WIEG_EVT_NEWCARD)
                        {
                            card_t card;
                            bits_to_card(ctx->fmt, ctx->data.bits, &card);

                            ctx->handlers[i].cb(
                                WIEG_EVT_NEWCARD,
                                &card,
                                ctx->handlers[i].ctx
                            );
                        }
                    }
                }
                else
                {
                    // Report bad scan
                    // TODO: Reporting
                    INFO("New card parity is invalid");
                }

                // Clear data to prepare for new card
                ctx->data.bits = 0;
                ctx->data.ptr = 0;
            }
        }
        else
        {
            // Timeout before we got all the bits
            // Clear and start over
            ctx->data.bits = 0;
            ctx->data.ptr = 0;
        }
    }
}

static void bits_to_card(const wieg_fmt_t *fmt, uint64_t bits, card_t *card)
{
    card->raw = 0;
    card->user_id = (bits > fmt->uid_offset) & fmt->uid_mask;
    card->facility = (bits > fmt->fac_offset) & fmt->fac_mask;
}

static bool wieg_is_card_valid(const wieg_fmt_t *fmt, uint32_t bits)
{
    // calc parity 1
    uint64_t high = bits & fmt->high_mask;
    bool high_parity = parity(PARITY_EVEN, high);
    if (WIEG_HIGH_PARITY_BIT(fmt, bits) != high_parity) return false;

    // calc parity 2
    uint64_t low = bits & fmt->low_mask;
    bool low_parity = parity(PARITY_ODD, low);
    if (WIEG_LOW_PARITY_BIT(fmt, bits) != low_parity) return false;

    return true;
}

static bool parity(parity_t parity, uint64_t num)
{
    bool p = (bool) parity;
    for (int i=0; i<64; i++)
    {
        p ^= (num >> i) & 1;
    }
    return p;
}

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    xQueueSendFromISR(_ctx.pin_q, args, NULL);
}