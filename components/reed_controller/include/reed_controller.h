#ifndef REED_CONTROLLER_H
#define REED_CONTROLLER_H

#define     REED_DIGITAL_INPUT       (22)

void reed_init(void (*interrupt_fn)());

#endif