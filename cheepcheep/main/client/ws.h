#ifndef WS_H_
#define WS_H_

#include "config.h"
#include "cJSON.h"

typedef enum {
    WS_OPEN,
    WS_CLOSE,
    WS_MSG,
} ws_evt_t;

typedef void (*ws_evt_cb_t)(ws_evt_t evt, cJSON *data, void *ctx);

status_t ws_init(const config_network_t *net_config);

status_t ws_start(char *uri);

status_t ws_send(cJSON *msg);

status_t ws_evt_cb_register(ws_evt_cb_t cb, void *ctx);

#endif /*WS_H_*/