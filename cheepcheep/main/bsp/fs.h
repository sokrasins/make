#ifndef STORAGE_H_
#define STORAGE_H_

#include "status.h"
#include <stddef.h>

typedef void *file_t;

status_t fs_init(void);

file_t fs_open(const char *name, const char *type);

status_t fs_read(file_t file, char *data, size_t chars);

status_t fs_readuntil(file_t file, char *data, char limit);

#define fs_readline(file, data) fs_readuntil(file, data, '\n')

status_t fs_write(file_t file, char *data, size_t chars);

void fs_rewind(file_t file);

status_t fs_close(file_t file);

status_t fs_rm(const char *name);

#endif /*STORAGE_H_*/