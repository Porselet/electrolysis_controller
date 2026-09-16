#include <Arduino.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include "ui.h"
#include "config.h"
#include "settings.h"

// ---------------------------------------------------------------------------
// LCD: пины KeyShield
// ---------------------------------------------------------------------------
static LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// ---------------------------------------------------------------------------
// Имена шагов
// ---------------------------------------------------------------------------
static const char* const STEP_NAMES[] = { "FILL", "HOLD", "VENT" };

// ---------------------------------------------------------------------------
// Форматирование
// ---------------------------------------------------------------------------
static void format_hms(char* buf, uint32_t t_s) {
    uint8_t hh = (t_s / 3600UL) % 100UL;
    uint8_t mm = (t_s / 60UL)   % 60UL;
    uint8_t ss =  t_s           % 60UL;
    sprintf(buf, "%02u:%02u:%02u", hh, mm, ss);
}

static void format_pressure(char* buf, float p) {
    dtostrf(p, 5, 1, buf);
}

// Затирает строку пробелами
static void clear_line(uint8_t row) {
    lcd.setCursor(0, row);
    lcd.print(F("                "));
}

// ---------------------------------------------------------------------------
// Экран 0: основной
// ---------------------------------------------------------------------------
void ui_draw_main(enum Step step, uint32_t timer_s,
                  float P, float dP, enum DegradedReason deg) {

    char tbuf[10];
    format_hms(tbuf, timer_s);

    lcd.setCursor(0, 0);
    lcd.print(STEP_NAMES[step]);
    lcd.print(F(" T="));
    lcd.print(tbuf);
    lcd.print(F("  "));

    lcd.setCursor(0, 1);

    if (deg == DEG_NONE) {
        char pbuf[8], dbuf[8];
        format_pressure(pbuf, P);
        format_pressure(dbuf, dP);
        lcd.print(F("P="));
        lcd.print(pbuf);
        lcd.print(F(" dP="));
        lcd.print(dbuf);
        lcd.print(F("  "));
    }
    else {
        char pbuf[8];
        format_pressure(pbuf, P);
        lcd.print(F("P="));
        lcd.print(pbuf);
        lcd.print(F("  "));
        lcd.print(deg == DEG_P1 ? F("P2 OPEN ") : F("P1 OPEN "));
    }
}

// ---------------------------------------------------------------------------
// Экран 1: датчики
// ---------------------------------------------------------------------------
static const char* const SENS_NAMES[] = {
    "OK", "OPEN", "SHORT", "RAIL_L", "RAIL_H"
};

static void draw_sensor_row(uint8_t row, char label,
                            const struct SensorReading* r) {
    lcd.setCursor(0, row);
    lcd.print(label);
    lcd.print('=');

    char pbuf[8];
    format_pressure(pbuf, r->bar);
    lcd.print(pbuf);
    lcd.print(' ');

    const char* s = SENS_NAMES[r->state];
    lcd.print(s);
    // затирка хвоста
    uint8_t len = 2 + 5 + 1 + strlen(s);
    for (uint8_t i = len; i < 16; i++) lcd.print(' ');
}

void ui_draw_sensors(const struct SensorReading* p1,
                     const struct SensorReading* p2) {
    draw_sensor_row(0, '1', p1);
    draw_sensor_row(1, '2', p2);
}

// ---------------------------------------------------------------------------
// Экран 2: уставки
// ---------------------------------------------------------------------------
void ui_draw_settings(uint8_t cursor) {
    char buf[10];

    // Строка 1: FILL
    lcd.setCursor(0, 0);
    lcd.print(cursor == 0 ? '>' : ' ');
    lcd.print(F("FILL "));
    format_pressure(buf, g_fill_threshold);
    lcd.print(buf);
    lcd.print(F(" atm  "));

    // Строка 2: HOLD (в минутах)
    uint32_t hold_min = g_hold_time_sec / 60UL;
    lcd.setCursor(0, 1);
    lcd.print(cursor == 1 ? '>' : ' ');
    lcd.print(F("HOLD "));
    // Вывод минут с выравниванием по правому краю (4 знака)
    if (hold_min > 9999) hold_min = 9999;
    if (hold_min < 10) lcd.print(' ');
    if (hold_min < 100) lcd.print(' ');
    if (hold_min < 1000) lcd.print(' ');
    lcd.print(hold_min);
    lcd.print(F(" min  "));
}

// ---------------------------------------------------------------------------
// Аварийный экран
// ---------------------------------------------------------------------------
void ui_draw_fault(float P, enum FaultReason fault) {
    lcd.setCursor(0, 0);
    lcd.print(F("!! FAULT !!     "));

    lcd.setCursor(0, 1);
    switch (fault) {
        case FAULT_OVER: {
            char pbuf[8], mbuf[8];
            format_pressure(pbuf, P);
            format_pressure(mbuf, P_MAX_BAR);
            lcd.print(F("OVER "));
            lcd.print(pbuf);
            lcd.print('/');
            lcd.print(mbuf);
            lcd.print(F("  "));
            break;
        }
        case FAULT_SENSOR:
            lcd.print(F("SENSOR FAULT    "));
            break;
        case FAULT_MISMATCH:
            lcd.print(F("P1!=P2 MISMATCH "));
            break;
        default:
            lcd.print(F("FAULT           "));
            break;
    }
}

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------
void ui_init(void) {
    lcd.begin(16, 2);
    lcd.clear();
}