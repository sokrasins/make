#ifndef STATUS_H_
#define STATUS_H_

#include <stdint.h>

// Generic status type
typedef uint32_t status_t;

// Status codes
#define STATUS_OK        0U
#define STATUS_ERR       1U
#define STATUS_IO        2U
#define STATUS_NOTIMPL   3U
#define STATUS_NOMEM     4U
#define STATUS_NXHANDLE  5U
#define STATUS_UNDEFINED 0xFFFFFFFF

#endif /*STATUS_H_*/