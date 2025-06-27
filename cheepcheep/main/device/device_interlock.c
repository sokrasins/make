#include "device_interlock.h"
#include "wiegand.h"

typedef struct {
    int id; // ": None,  # any value == on, None == off
    int kwh; //": 0,
} interlock_session_t;

typedef struct {
    const config_general_t *config;
    wieg_evt_handle_t evt_handle;
    interlock_session_t session;
} ilock_ctx_t;

static status_t interlock_init(const config_t *config);
static void interlock_handle_swipe(wieg_evt_t event, card_t *card, void *ctx);

device_t interlock = {
    .init = interlock_init,
};

static ilock_ctx_t _ctx;

static status_t interlock_init(const config_t *config)
{
    _ctx.evt_handle = wieg_evt_handler_reg(WIEG_EVT_NEWCARD, interlock_handle_swipe, (void *)&_ctx);
    return -STATUS_UNIMPL;
}

static void interlock_handle_swipe(wieg_evt_t event, card_t *card, void *ctx)
{

}
