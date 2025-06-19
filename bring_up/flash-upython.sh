#!/bin/sh

# Serial port of the ESP
port=$1

# Default micropython binary
upython_bin="ESP32_GENERIC_S3-FLASH_4M-20241129-v1.24.1.bin"

# Make sure we got an argument for an existing port
if [ -z "$port" ]; then
  echo "ERROR: Must provide a serial port (example: ${0} /dev/ttyUSB0)"
  exit 1
fi

# If provided, replace the default micropython binary path with the supplied one
if [ ! -z $2 ]; then
  upython_bin=$2
fi

# Make sure serial port exists
if [ ! -e $port ]; then
  echo "ERROR: port ${port} does not exist"
  exit 2
fi

# Make sure the micropython binary exists too
if [ ! -e $upython_bin ]; then
  echo "ERROR: binary ${upython_bin} does not exist"
  exit 3
fi

echo "Writing ${upython_bin} to ${port}"

# Erase the flash first
python -m esptool --chip esp32s3 --port $port erase_flash

# Then write the bin
python -m esptool --chip esp32s3 --port $port write_flash -z 0 $upython_bin
