#include "fs.h"
#include "esp_littlefs.h"
#include "log.h"

#include <stdio.h>

#define FS_BASE_PATH       "/fs"
#define FS_PARTITION_LABEL "storage"

status_t fs_init(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path              = FS_BASE_PATH,
        .partition_label        = FS_PARTITION_LABEL,
        .format_if_mount_failed = true,
        .dont_mount             = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ERROR("Failed to mount or format filesystem");
        } 
        else if (ret == ESP_ERR_NOT_FOUND) 
        {
            ERROR("Failed to find LittleFS partition");
        } 
        else 
        {
            ERROR("Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return -STATUS_IO;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) 
    {
        ERROR("Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } 
    else 
    {
        INFO("Partition size: total: %d, used: %d", total, used);
    }

    return STATUS_OK;
}

file_t fs_open(const char *name, const char *type)
{
    char path[64];
    int rc = sprintf(path, "%s/%s", FS_BASE_PATH, name);
    return (file_t) fopen(path, type);
}

status_t fs_read(char *data, size_t chars, file_t file)
{
    char *ret = fgets(data, chars, (FILE *) file);
    if (ret != data)
    {
        return -STATUS_IO;
    }

    return STATUS_OK;
}

status_t fs_close(file_t file)
{
    int rc = fclose((FILE *) file);
    return rc == 0 ? STATUS_OK : -STATUS_NOFILE;
}

status_t fs_rm(const char *name)
{
    int rc = remove(name);
    if (rc != 0)
    {
        return -STATUS_NOFILE;
    }

    return STATUS_OK;
}