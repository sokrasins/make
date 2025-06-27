#ifndef CLIENT_H_
#define CLIENT_H_

#include "status.h"
#include "config.h"
#include "msg.h"
#include <cJSON.h>

typedef status_t (*client_cmd_handler_t)(msg_t *msg);

status_t client_init(device_type_t device, const config_portal_t *portal_config, const config_network_t *net_config);

status_t client_handler_register(client_cmd_handler_t handler);

status_t client_send_msg(msg_t *msg);

#endif /*CLIENT_H_*/