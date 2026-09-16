#include <Arduino.h>
#include "buttons.h"
#include "config.h"

static enum Button last_stable    = BTN_NONE;
static enum Button last_raw       = BTN_NONE;
static uint32_t    last_change_ms = 0;

static enum Button read_raw(void) {
    int v = analogRead(PIN_BUTTONS);
    if (v < BTN_ADC_RIGHT_MAX)  return BTN_RIGHT;
    if (v < BTN_ADC_UP_MAX)     return BTN_UP;
    if (v < BTN_ADC_DOWN_MAX)   return BTN_DOWN;
    if (v < BTN_ADC_LEFT_MAX)   return BTN_LEFT;
    if (v < BTN_ADC_SELECT_MAX) return BTN_SELECT;
    return BTN_NONE;
}

void buttons_init(void) {
    last_stable    = read_raw();
    last_raw       = last_stable;
    last_change_ms = millis();
}

enum Button buttons_poll(void) {
    enum Button raw = read_raw();
    uint32_t now = millis();

    // Изменилось «сырое» состояние — перезапускаем антидребезг
    if (raw != last_raw) {
        last_raw       = raw;
        last_change_ms = now;
        return BTN_NONE;
    }

    // Ещё не прошло время стабилизации
    if ((now - last_change_ms) < BTN_DEBOUNCE_MS) return BTN_NONE;

    // Стабильное состояние сменилось
    if (raw != last_stable) {
        enum Button prev = last_stable;
        last_stable = raw;
        // Возвращаем только фронт нажатия (NONE -> кнопка)
        if (prev == BTN_NONE && raw != BTN_NONE) {
            return raw;
        }
    }

    return BTN_NONE;
}