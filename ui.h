#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "algo.h"
#include "safety.h"

void ui_init(void);
void ui_update(enum Step step, uint32_t timer_s,
               float P, float dP,
               enum DegradedReason deg,
               enum FaultReason fault);

#endif