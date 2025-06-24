#ifndef STORAGE_H_
#define STORAGE_H_

#include "status.h"
#include <stddef.h>

typedef void *file_t;

status_t fs_init(void);

file_t fs_open(const char *name, const char *type);

status_t fs_read(char *data, size_t chars, file_t file);

status_t fs_close(file_t file);

status_t fs_rm(const char *name);

#endif /*STORAGE_H_*/