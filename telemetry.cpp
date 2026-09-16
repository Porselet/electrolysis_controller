#include <Arduino.h>
#include "telemetry.h"
#include "config.h"

void telemetry_init(void) {
#if TELEMETRY_ENABLED
    Serial.begin(115200);
    Serial.println(F("=== electrolysis controller ==="));
#endif
}

void telemetry_periodic(enum Step step, uint32_t timer_s,
                        float P, float dP,
                        bool v1, bool v2,
                        enum DegradedReason deg) {
#if TELEMETRY_ENABLED
    static const char* const names[] = { "FILL", "HOLD", "VENT" };
    uint8_t hh = (timer_s / 3600UL) % 100UL;
    uint8_t mm = (timer_s / 60UL)   % 60UL;
    uint8_t ss =  timer_s           % 60UL;

    Serial.print(F("T="));
    Serial.print(millis());
    Serial.print(' ');
    Serial.print(names[step]);
    Serial.print(F(" t="));
    if (hh < 10) Serial.print('0');
    Serial.print(hh);
    Serial.print(':');
    if (mm < 10) Serial.print('0');
    Serial.print(mm);
    Serial.print(':');
    if (ss < 10) Serial.print('0');
    Serial.print(ss);
    Serial.print(F(" P="));
    Serial.print(P, 1);
    Serial.print(F(" dP="));
    Serial.print(dP, 1);
    Serial.print(F(" V1="));
    Serial.print(v1 ? 1 : 0);
    Serial.print(F(" V2="));
    Serial.print(v2 ? 1 : 0);
    if (deg == DEG_P1) Serial.print(F(" DEG_P1"));
    if (deg == DEG_P2) Serial.print(F(" DEG_P2"));
    Serial.println();
#endif
}

void telemetry_event(const char* msg) {
#if TELEMETRY_ENABLED
    Serial.print(F("[EVENT] "));
    Serial.println(msg);
#endif
}