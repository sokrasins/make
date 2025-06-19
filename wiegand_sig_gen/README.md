# Wiegand Signal Generator

A signal generator for the Wiegand protocol on the Raspberry Pi pico. The generator is capable of generating 26-bit signals with arbitrary pulse characteristics. The sig gen is controlled through a UART interface.

## Building

1. Install Raspberry Pi extension in vscode
2. Install Raspberry pi pico SDK (through the extension)
3. Import this project
4. Build and run

## Usage

Connect FTDI cable to uart on pins 4 and 5

Connect pins 2 and 3 are the wiegand-generating pins (D0 and D1, respectively).

The FTDI interface lets you configure the generator and send signals. The commands are:

| Command | Function | Example |
| ------- | -------- | ------- |
| ECHO [0|1] | Sets terminal echo on (if 1) or off (if 0) | `ECHO 1` |
| PULSE <width> <duration> | Sets pulse characteristics (in microseconds) | `PULSE 350 1528` | SEND <facility> <user_id> | Generates wiegand card data | `SEND 12 3456` |
| R <times> | Repeat the last card data each second for a fixed number of times | `R 10`

When commands are sent, one of two responses will be sent:

- `OK`: The command was accepted and executed
- `KO`: The command was unrecognized, or a command parameter was invalid

