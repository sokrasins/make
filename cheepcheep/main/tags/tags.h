#ifndef TAGS_H_
#define TAGS_H_

#include "status.h"
#include <stddef.h>

status_t tags_init(void);

status_t tags_find(uint32_t raw);

status_t tags_update(uint32_t *tags, size_t num_tags, uint8_t hash);

#endif /*TAGS_H_*/