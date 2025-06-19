#ifndef BSP_H_
#define BSP_H_

#include "hardware/uart.h"
#include "hardware/gpio.h"

// UART defines
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// Use pins 2 and 3 for wiegand emulation
#define WIEGAND_D0_PIN 2
#define WIEGAND_D1_PIN 3

#endif /*BSP_H_*/