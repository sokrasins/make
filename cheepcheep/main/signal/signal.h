#ifndef BUZZER_H_
#define BUZZER_H_

#include "status.h"
#include "config.h"

status_t signal_init(const config_buzzer_t *config);

void signal_alert(void);
void signal_ok(void);
void signal_cardread(void);
void signal_action(void);

#endif /*BUZZER_H_*/