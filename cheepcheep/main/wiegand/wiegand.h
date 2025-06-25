#ifndef WIEGAND_H_
#define WIEGAND_H_

#include "status.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    WIEG_EVT_NEWCARD,
    WIEG_EVT_NEWBIT,
} wieg_evt_t;

typedef enum {
    WIEG_26_BIT,
    WIEG_34_BIT,
} wieg_encoding_t;

typedef union {
    uint32_t raw;
    struct __attribute__((packed)) {
        uint16_t user_id;
        uint16_t facility;
    };
} card_t;

typedef void *wieg_evt_handle_t;

typedef void (*wieg_evt_cb_t)(wieg_evt_t event, card_t *card, void *ctx);

status_t wieg_init(int d0, int d1, wieg_encoding_t encode);

wieg_evt_handle_t wieg_evt_handler_reg(wieg_evt_t event, wieg_evt_cb_t cb, void *ctx);

status_t wieg_evt_handler_dereg(wieg_evt_handle_t handle);

#endif /*WIEGAND_H_*/