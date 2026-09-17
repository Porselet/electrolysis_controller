#ifndef ALGO_H
#define ALGO_H

#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------
// Шаги алгоритма
// ---------------------------------------------------------------------------
enum Step
{
    S_FILL = 0,
    S_HOLD,
    S_VENT
};

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------
void algo_reset(void);
void algo_tick(uint32_t dt_ms, float P,
               bool *v1, bool *v2, enum Step *step,
               uint32_t *timer_ms_out);

#endif