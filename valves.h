#ifndef VALVES_H
#define VALVES_H

#include <stdbool.h>

void valves_init(void);
void valves_set(bool v1, bool v2);
void valves_alarm(bool on);

#endif