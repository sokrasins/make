#ifndef NVSTATE_H_
#define NVSTATE_H_

#include "status.h"
#include <stddef.h>
#include <stdbool.h>

status_t nvstate_init(void);

bool nvstate_locked_out(void);
status_t nvstate_locked_out_set(bool locked_out);

status_t nvstate_tag_hash(char *tag_hash, size_t *len);
status_t nvstate_tag_hash_set(char *tag_hash, size_t len);

#endif /*NVSTATE_H_*/