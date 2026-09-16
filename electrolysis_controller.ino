#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "valves.h"
#include "algo.h"
#include "safety.h"
#include "ui.h"
#include "telemetry.h"
#include "settings.h"
#include "buttons.h"

// ---------------------------------------------------------------------------
// Состояние главного цикла
// ---------------------------------------------------------------------------
static uint32_t last_ms       = 0;
static uint32_t last_ui_ms    = 0;
static uint32_t last_telem_ms = 0;
static bool     prev_v2       = false;
static enum Step last_logged_step = (enum Step)255;

// ---------------------------------------------------------------------------
// Состояние UI
// ---------------------------------------------------------------------------
static enum UiScreen screen = UI_MAIN;
static uint8_t       cursor = 0;   // 0 = FILL, 1 = HOLD

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
    valves_init();
    sensors_init();
    ui_init();
    telemetry_init();
    buttons_init();

    settings_load();
    algo_reset();

    last_ms = millis();
    last_logged_step = S_FILL;
    telemetry_event("boot");
}

// ---------------------------------------------------------------------------
// Обработка кнопок
// ---------------------------------------------------------------------------
static void handle_button(enum Button b) {
    if (b == BTN_NONE) return;

    if (screen == UI_MAIN || screen == UI_SENSORS) {
        // На экранах 0 и 1 работает только Select — листает дальше
        if (b == BTN_SELECT) {
            screen = (enum UiScreen)((screen + 1) % 3);
        }
        return;
    }

    // Экран 2 (уставки)
    switch (b) {
        case BTN_SELECT:
            settings_save_now();
            screen = UI_MAIN;
            break;

        case BTN_LEFT:
            cursor = 0;
            break;

        case BTN_RIGHT:
            cursor = 1;
            break;

        case BTN_UP:
            if (cursor == 0) {
                g_fill_threshold += FILL_STEP_ATM;
                if (g_fill_threshold > FILL_MAX_ATM) g_fill_threshold = FILL_MAX_ATM;
                settings_mark_dirty();
            } else {
                uint32_t mins = g_hold_time_sec / 60UL + HOLD_STEP_MIN;
                if (mins > HOLD_MAX_MIN) mins = HOLD_MAX_MIN;
                g_hold_time_sec = mins * 60UL;
                settings_mark_dirty();
            }
            break;

        case BTN_DOWN:
            if (cursor == 0) {
                g_fill_threshold -= FILL_STEP_ATM;
                if (g_fill_threshold < FILL_MIN_ATM) g_fill_threshold = FILL_MIN_ATM;
                settings_mark_dirty();
            } else {
                uint32_t mins = g_hold_time_sec / 60UL;
                if (mins >= HOLD_STEP_MIN) {
                    mins -= HOLD_STEP_MIN;
                } else {
                    mins = HOLD_MIN_MIN;
                }
                g_hold_time_sec = mins * 60UL;
                settings_mark_dirty();
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------------
void loop() {
    uint32_t now = millis();
    uint32_t dt  = now - last_ms;
    last_ms = now;

    // 0. Автосохранение уставок
    settings_tick(dt);

    // 1. Кнопки
    handle_button(buttons_poll());

    // 2. Датчики
    struct SensorReading p1, p2;
    sensors_read(&p1, &p2);

    // 3. Безопасность
    struct SafetyResult sr;
    safety_check(&p1, &p1, &sr);

    // 4. Авария — терминальное состояние
    if (sr.fault != FAULT_NONE) {
        bool v1, v2;
        safety_apply_fault(sr.fault, prev_v2, &v1, &v2);
        valves_set(v1, v2);
        valves_alarm(true);
        ui_draw_fault(sr.P, sr.fault);
        telemetry_event("FAULT");
        while (1) { /* ждём перезапуска питания */ }
    }

    // 5. Алгоритм
    enum Step step;
    bool      v1, v2;
    uint32_t  t_ms;
    algo_tick(dt, sr.P, &v1, &v2, &step, &t_ms);
    uint32_t step_timer_s = t_ms / 1000;

    if (step != last_logged_step) {
        last_logged_step = step;
        telemetry_event("step change");
    }

    // 6. Клапаны
    valves_set(v1, v2);
    prev_v2 = v2;

    // 7. UI
    if (now - last_ui_ms >= UI_PERIOD_MS) {
        last_ui_ms = now;

        switch (screen) {
            case UI_MAIN:
                ui_draw_main(step, step_timer_s, sr.P, sr.dP, sr.degraded);
                break;
            case UI_SENSORS:
                ui_draw_sensors(&p1, &p2);
                break;
            case UI_SETTINGS:
                ui_draw_settings(cursor);
                break;
        }
    }

    // 8. Telemetry
    if (now - last_telem_ms >= TELEMETRY_PERIOD_MS) {
        last_telem_ms = now;
        telemetry_periodic(step, step_timer_s, sr.P, sr.dP, v1, v2, sr.degraded);
    }
}