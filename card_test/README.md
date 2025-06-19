# Card Test

This micropython test strips all wifi connectivity out of the stock BeepBeep application. The only thing left is the wiegand card reading loop. When a card is read, it's logged to the serial port. This can be used in conjunction with the test utility in this repo.

## Installing

Micropython is required to be running on the device before installation (see the bringup section in this repo).

1. Connect to the device with an FTDI cable (connecting TX, RX and GND... but NOT Vcc).
2. Power the board. Verify micropython is running and interactable (e.g. python REPL works and no script is running)
3. Run the test loader in this folder: `./write-test.sh /dev/ttyUSB0`
4. Power-cycle the board. When the board receives card data it will be logged to UART0

