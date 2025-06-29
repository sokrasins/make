#ifndef STATUS_H_
#define STATUS_H_

#include <stdint.h>

typedef int32_t status_t;

#define STATUS_OK 0
#define STATUS_UNIMPL 1
#define STATUS_NOMEM 2
#define STATUS_UNAVAILABLE 3
#define STATUS_IO 4
#define STATUS_NOFILE 5
#define STATUS_PARSE 6
#define STATUS_INVALID 7
#define STATUS_INVAL 8
#define STATUS_EOF 9
#define STATUS_NO_RESOURCE 10

#endif /*STATUS_H_*/