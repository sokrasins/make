#ifndef OTA_DFU_H_
#define OTA_DFU_H_

#include "status.h"
#include "config.h"
#include <stdbool.h>

status_t ota_dfu_init(const config_dfu_t *config);

status_t ota_mark_application(bool valid);

#endif /*OTA_DFU_H_*/