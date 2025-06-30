#ifndef OTA_DFU_H_
#define OTA_DFU_H_

#include "status.h"
#include "config.h"

status_t ota_dfu_init(const config_dfu_t *config);

#endif /*OTA_DFU_H_*/