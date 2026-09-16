#include "algo.h"
#include "config.h"

// ---------------------------------------------------------------------------
// Состояние автомата
// ---------------------------------------------------------------------------
static enum Step state;
static uint32_t  timer_ms;

// ---------------------------------------------------------------------------
// Макросы для описания шагов
// ---------------------------------------------------------------------------
#define STEP(name)                                    \
    case S_##name: {                                  \
        bool __fire = false;                          \
        (void)__fire;                                 \
        enum Step __next = state;                     \
        (void)__next;

#define END_STEP                                      \
        if (__fire) { state = __next; timer_ms = 0; } \
    } break;

#define V1(v)         (*v1_out = (v))
#define V2(v)         (*v2_out = (v))
#define OPEN          true
#define CLOSE         false

#define WAIT_UNTIL(c) if (c) __fire = true
#define WAIT_SECONDS(s) \
    if (timer_ms >= (uint32_t)((s) * 1000UL)) __fire = true

#define NEXT(n)       __next = S_##n

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------
void algo_reset(void) {
    state    = S_FILL;
    timer_ms = 0;
}

void algo_tick(uint32_t dt_ms, float P,
               bool* v1_out, bool* v2_out, enum Step* step_out,
               uint32_t* timer_ms_out) {

    timer_ms += dt_ms;

    switch (state) {

        // Накачка: V1 открыт, V2 закрыт. Ждём P >= FILL_THRESHOLD.
        STEP(FILL) {
            V1(OPEN); V2(CLOSE);
            WAIT_UNTIL(P >= FILL_THRESHOLD);
            NEXT(HOLD);
        } END_STEP

        // Выдержка: оба закрыты. Ждём HOLD_TIME_SEC.
        STEP(HOLD) {
            V1(CLOSE); V2(CLOSE);
            WAIT_SECONDS(HOLD_TIME_SEC);
            NEXT(VENT);
        } END_STEP

        // Сброс: V1 закрыт, V2 открыт. Ждём P <= VENT_THRESHOLD.
        STEP(VENT) {
            V1(CLOSE); V2(OPEN);
            WAIT_UNTIL(P <= VENT_THRESHOLD);
            NEXT(FILL);
        } END_STEP
    }

    *step_out     = state;
    *timer_ms_out = timer_ms;
}