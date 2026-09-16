#include <Arduino.h>
#include <LiquidCrystal.h>
#include "ui.h"
#include "config.h"

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
    // XX.X — фиксированная ширина
    ///sprintf(buf, "%5.1f", p);
    dtostrf(p, 5, 1, buf);   // ширина 5, 1 знак после запятой
}

// ---------------------------------------------------------------------------
// Рабочий экран
// ---------------------------------------------------------------------------
static void draw_work(enum Step step, uint32_t timer_s,
                      float P, float dP, enum DegradedReason deg) {

    char tbuf[10];
    format_hms(tbuf, timer_s);

    lcd.setCursor(0, 0);
    lcd.print(STEP_NAMES[step]);
    lcd.print(" T=");
    lcd.print(tbuf);
    lcd.print("  ");   // затираем хвост

    lcd.setCursor(0, 1);

    if (deg == DEG_NONE) {
        char pbuf[8], dbuf[8];
        format_pressure(pbuf, P);
        format_pressure(dbuf, dP);
        lcd.print("P=");
        lcd.print(pbuf);
        lcd.print(" dP=");
        lcd.print(dbuf);
        lcd.print("  ");
    }
    else if (deg == DEG_P1) {
        char pbuf[8];
        format_pressure(pbuf, P);
        lcd.print("P=");
        lcd.print(pbuf);
        lcd.print(" P2 OPEN ");
    }
    else if (deg == DEG_P2) {
        char pbuf[8];
        format_pressure(pbuf, P);
        lcd.print("P=");
        lcd.print(pbuf);
        lcd.print(" P1 OPEN ");
    }
}

// ---------------------------------------------------------------------------
// Аварийный экран
// ---------------------------------------------------------------------------
static void draw_fault(float P, enum FaultReason fault) {
    lcd.setCursor(0, 0);
    lcd.print("!! FAULT !!     ");

    lcd.setCursor(0, 1);
    switch (fault) {
        case FAULT_OVER: {
            char pbuf[8];
            format_pressure(pbuf, P);
            lcd.print("OVER ");
            lcd.print(pbuf);
            lcd.print("/");
            char mbuf[8];
            format_pressure(mbuf, P_MAX_BAR);
            lcd.print(mbuf);
            lcd.print(" ");
            break;
        }
        case FAULT_SENSOR:
            lcd.print("SENSOR FAULT    ");
            break;
        case FAULT_MISMATCH:
            lcd.print("P1!=P2 MISMATCH ");
            break;
        default:
            lcd.print("FAULT           ");
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

void ui_update(enum Step step, uint32_t timer_s,
               float P, float dP,
               enum DegradedReason deg,
               enum FaultReason fault) {

    if (fault != FAULT_NONE) {
        draw_fault(P, fault);
    } else {
        draw_work(step, timer_s, P, dP, deg);
    }
}