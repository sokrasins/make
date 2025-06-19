#!/bin/sh
# Load the test python to the micropython target
# Usage: ./write_test.sh /dev/ttyUSB0

port=$1

load() {
  echo "Loading $2"
  ampy -p $1 put $2
}

echo "Loading test file to port $port"

# Necessary files and folders
load $port boot.py
load $port main.py
load $port config.py
load $port hardware.py
load $port leds.py
load $port utils.py
load $port ulcdscreen
load $port uwebsockets
load $port uwiegand
load $port ulogging
load $port configuration
