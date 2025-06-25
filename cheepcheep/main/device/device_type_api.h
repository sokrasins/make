#ifndef DEVICE_TYPE_API_H_
#define DEVICE_TYPE_API_H_

#include "config_types.h"
#include "status.h"

typedef status_t (*device_init_t)(const config_t *config);

typedef struct {
    device_init_t init;
} device_t;

#endif /*DEVICE_TYPE_API_H_*/