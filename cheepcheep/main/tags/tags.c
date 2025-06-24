#include "tags.h"
#include "bsp.h"

file_t tag_file;

status_t tags_init(void)
{
    tag_file = fs_open("tags.json", "r");

    // TODO: verify hash

    return STATUS_OK;
}

status_t tags_find(uint32_t raw)
{
    // approve all cards for debug
    return STATUS_OK;
}