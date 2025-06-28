#ifndef TAGS_H_
#define TAGS_H_

#include "status.h"
#include <stddef.h>

status_t tags_init(void);

status_t tags_find(uint32_t raw);

#endif /*TAGS_H_*/