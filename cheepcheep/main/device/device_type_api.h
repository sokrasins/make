#ifndef DEVICE_TYPE_API_H_
#define DEVICE_TYPE_API_H_

#include "config_types.h"
#include "status.h"
#include "wiegand.h"

typedef status_t (*device_init_t)(const config_t *config);

typedef struct {
    device_init_t init;
    wieg_evt_cb_t swipe_cb;
} device_t;

#endif /*DEVICE_TYPE_API_H_*/