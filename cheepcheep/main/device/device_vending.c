#include "device_vending.h"
#include "wiegand.h"

typedef struct {
    const config_general_t *config;
    wieg_evt_handle_t evt_handle;
} vending_ctx_t;

static status_t vending_init(const config_t *config);
static void vending_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

device_t vending = {
    .init = vending_init,
};

static vending_ctx_t _ctx;

static status_t vending_init(const config_t *config)
{
    _ctx.evt_handle = wieg_evt_handler_reg(WIEG_EVT_NEWCARD, vending_handle_swipe, (void *)&_ctx);
    return -STATUS_UNIMPL;
}

static void vending_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{

}