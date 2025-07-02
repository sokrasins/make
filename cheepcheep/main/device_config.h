#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

#include "config_types.h"
#include "device_creds.h"

#define CONFIG_DEVICE_TYPE                  DEVICE_DOOR       
#define CONFIG_DFU_SKIP_CN_CHECK            true
#define CONFIG_DFU_SKIP_VERSION_CHECK       false
#define CONFIG_GEN_FIXED_UNLOCK_DELAY       4
#define CONFIG_DEV_LOG_LEVEL                LOG_DEBUG

#endif /*DEVICE_CONFIG_H_*/