#ifndef NVSTATE_H_
#define NVSTATE_H_

#include "status.h"
#include "config.h"
#include <stddef.h>
#include <stdbool.h>

#define TAG_HASH_LEN 16U

status_t nvstate_init(void);

bool nvstate_locked_out(void);
status_t nvstate_locked_out_set(bool locked_out);

status_t nvstate_tag_hash(uint8_t *tag_hash, size_t *len);
status_t nvstate_tag_hash_set(uint8_t *tag_hash, size_t len);

status_t nvstate_config(config_t *config);
status_t nvstate_config_set(const config_t *config);

#endif /*NVSTATE_H_*/