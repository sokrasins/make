#include "device_interlock.h"

typedef struct {
    int id; // ": None,  # any value == on, None == off
    int kwh; //": 0,
} interlock_session_t;

static status_t interlock_init(const config_t *config);
static void interlock_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

device_t interlock = {
    .init = interlock_init,
    .swipe_cb = interlock_handle_swipe,
};

static interlock_session_t _session = {
    .id = 0,
    .kwh = 0,
};

static status_t interlock_init(const config_t *config)
{
    return -STATUS_UNIMPL;
}

static void interlock_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{

}
