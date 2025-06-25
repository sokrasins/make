#ifndef NET_H_
#define NET_H_

#include "status.h"
#include "config.h"

status_t net_init(const config_network_t *config);

void net_get_mac(uint8_t *mac);

#endif /*NET_H_*/