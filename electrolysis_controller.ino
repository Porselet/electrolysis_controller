#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "valves.h"
#include "algo.h"
#include "safety.h"
#include "ui.h"
#include "telemetry.h"

// ---------------------------------------------------------------------------
// Состояние главного цикла
// ---------------------------------------------------------------------------
static uint32_t last_ms       = 0;
static uint32_t last_ui_ms    = 0;
static uint32_t last_telem_ms = 0;
static bool     prev_v2       = false;
static enum Step last_logged_step = (enum Step)255;

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
    valves_init();
    sensors_init();
    ui_init();
    telemetry_init();
    algo_reset();

    last_ms = millis();
    telemetry_event("boot");
}

// ---------------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------------
void loop() {
    uint32_t now = millis();
    uint32_t dt  = now - last_ms;
    last_ms = now;

    // 1. Датчики
    struct SensorReading p1, p2;
    sensors_read(&p1, &p2);

    // 2. Безопасность
    struct SafetyResult sr;
    safety_check(&p1, &p1, &sr);

    // 3. Авария — терминальное состояние
    if (sr.fault != FAULT_NONE) {
        bool v1, v2;
        safety_apply_fault(sr.fault, prev_v2, &v1, &v2);
        valves_set(v1, v2);
        valves_alarm(true);
        ui_update(S_FILL, 0, sr.P, sr.dP, sr.degraded, sr.fault);
        telemetry_event("FAULT");
        while (1) { /* ждём перезапуска питания */ }
    }

    // 4. Алгоритм
    enum Step step;
    bool      v1, v2;
    uint32_t  t_ms;
    algo_tick(dt, sr.P, &v1, &v2, &step, &t_ms);
    uint32_t step_timer_s = t_ms / 1000;

    // Лог смены шага (один раз на переход)
    if (step != last_logged_step) {
        last_logged_step = step;
        telemetry_event("step change");
    }

    // 5. Вывод на клапаны
    valves_set(v1, v2);
    prev_v2 = v2;

    // 6. UI
    if (now - last_ui_ms >= UI_PERIOD_MS) {
        last_ui_ms = now;
        ui_update(step, step_timer_s, sr.P, sr.dP, sr.degraded, FAULT_NONE);
    }

    // 7. Telemetry
    if (now - last_telem_ms >= TELEMETRY_PERIOD_MS) {
        last_telem_ms = now;
        telemetry_periodic(step, step_timer_s, sr.P, sr.dP, v1, v2, sr.degraded);
    }
}