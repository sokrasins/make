#ifndef BUZZER_H_
#define BUZZER_H_

#include "status.h"

status_t signal_init(void);

void signal_alert(void);
void signal_ok(void);

#endif /*BUZZER_H_*/