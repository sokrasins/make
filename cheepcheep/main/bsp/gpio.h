#ifndef GPIO_H_
#define GPIO_H_

#include "config.h"
#include "status.h"

#include <stdbool.h>

typedef enum {
    INPUT_AUX1 = 0,
    INPUT_AUX2,
    INPUT_DOOR_SENSOR,
    INPUT_IN1,
    INPUT_INVAL,
} input_t;

typedef enum {
    OUTPUT_STATUS_LED = 0,
    OUTPUT_READER_LED,
    OUTPUT_READER_BUZZER,
    OUTPUT_RELAY,
    OUTPUT_LOCK,
    OUTPUT_OUT1,
    OUTPUT_INVAL,
} output_t;

status_t gpio_init(const config_pins_t *pins, const config_general_t *gen);

status_t gpio_out_set(output_t out, bool state);

int gpio_in_get(input_t in);

#endif /*GPIO_H_*/