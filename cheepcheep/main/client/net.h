#ifndef NET_H_
#define NET_H_

#include "status.h"
#include "config.h"

typedef enum {
    NET_EVT_DISCONNECT,
    NET_EVT_CONNECT,
} net_evt_t;

typedef void (*net_evt_cb_t)(net_evt_t evt, void *ctx);
typedef void *net_evt_handle_t;

status_t net_init(const config_network_t *config);

status_t net_connect(void);

net_evt_handle_t net_evt_cb_register(net_evt_t evt, void *ctx, net_evt_cb_t cb);

void net_get_mac(uint8_t *mac);

uint32_t net_get_ip(void);

#endif /*NET_H_*/