#include "device_vending.h"

static status_t vending_init(const config_t *config);
static void vending_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

device_t vending = {
    .init = vending_init,
    .swipe_cb = vending_handle_swipe,
};

static status_t vending_init(const config_t *config)
{
    return -STATUS_UNIMPL;
}

static void vending_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{

}