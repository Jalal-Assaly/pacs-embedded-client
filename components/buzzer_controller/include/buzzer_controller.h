#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#define     BUZZER      (23)

void buzzer_init();
void buzzer_on();
void buzzer_off();
void buzzer_onFor(unsigned long duration_ms);

#endif
