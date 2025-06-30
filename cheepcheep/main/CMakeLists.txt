idf_component_register(
SRCS 
    "main.c"
    "bsp/fs.c"
    "bsp/gpio.c"
    "bsp/uptime.c"
    "config/config.c"
    "wiegand/wiegand.c"
    "wiegand/wiegand_fmt.c"
    "nvstate/nvstate.c"
    "device/device_door.c"
    "device/device_interlock.c"
    "device/device_vending.c"
    "tags/tags.c"
    "signal/signal.c"
    "client/net.c"
    "client/ws.c"
    "client/client.c"
    "client/msg.c"
    "client/ota_dfu.c"
INCLUDE_DIRS 
    "."
    "wiegand"
    "bsp"
    "util"
    "config"
    "nvstate"
    "device"
    "tags"
    "signal"
    "client"
    )

# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
littlefs_create_partition_image(storage ../flash_data FLASH_IN_PROJECT)