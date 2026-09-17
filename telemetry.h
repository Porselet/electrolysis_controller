#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include "algo.h"
#include "safety.h"

void telemetry_init(void);
void telemetry_periodic(enum Step step, uint32_t timer_s,
                        float P, float dP,
                        bool v1, bool v2,
                        enum DegradedReason deg);
void telemetry_event(const char *msg);

#endif