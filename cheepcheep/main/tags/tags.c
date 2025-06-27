#include "tags.h"
#include "bsp.h"
#include "client.h"
#include "nvstate.h"
#include "log.h"

#include "cJSON.h"

#include <stdio.h>

status_t tag_sync_handler(msg_t *msg);

file_t tag_file;

status_t tags_init(void)
{
    tag_file = fs_open("tags.json", "r");

    // TODO: verify hash
    return client_handler_register(tag_sync_handler);
}

status_t tags_find(uint32_t raw)
{
    // approve all cards for debug
    char card_str[16];
    status_t status = fs_readline(tag_file, card_str);
    while (status != -STATUS_EOF) 
    {
        uint32_t db_card = atoi(card_str);
        if (db_card == raw)
        {
            fs_rewind(tag_file);
            return STATUS_OK;
        }
        status = fs_readline(tag_file, card_str);
    }

    fs_rewind(tag_file);
    return -STATUS_INVALID;
}

status_t tag_sync_handler(msg_t *msg)
{
    if (msg->type == MSG_SYNC)
    {
        // Get the existing hash
        // TODO: Doesn't work
        // TODO: Calculate instead of storing
        uint8_t cur_hash[TAG_HASH_LEN];
        size_t hash_len;
        nvstate_tag_hash(cur_hash, &hash_len);

        // Only update the tags if the hashes are different
        if (memcmp(cur_hash, msg->sync.hash, TAG_HASH_LEN) != 0)
        {
            WARN("New tag list received, saving...");
            char file_line[16];

            status_t status = fs_close(tag_file);
            status = fs_rm("tags.json");
            file_t new_file = fs_open("tags.json", "w");
            if (new_file == 0)
            {
                ERROR("Couldn't save new tags");
                return STATUS_OK;
            }

            cJSON *tag;
            cJSON_ArrayForEach(tag, msg->sync.tags)
            {
                int len = sprintf(file_line, "%d\n", atoi(tag->valuestring));
                status = fs_write(new_file, file_line, len);
            }

            // Roopen the file as read-only
            fs_close(new_file);
            tag_file = fs_open("tags.json", "r");

            // Store the curernt hash
            // TODO: Calculate in the future?
            nvstate_tag_hash_set(msg->sync.hash, TAG_HASH_LEN);

            WARN("Done saving tags");
        }
        else
        {
            WARN("Tag list is the same, skipping");
        }
        return STATUS_OK;
    }
    return -STATUS_UNAVAILABLE;
}