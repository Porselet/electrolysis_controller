#include <Arduino.h>
#include "valves.h"
#include "config.h"

void valves_init(void)
{
    pinMode(PIN_V1, OUTPUT);
    pinMode(PIN_V2, OUTPUT);
    pinMode(PIN_ALARM, OUTPUT);

    digitalWrite(PIN_V1, LOW);
    digitalWrite(PIN_V2, LOW);
    digitalWrite(PIN_ALARM, LOW);
}

void valves_set(bool v1, bool v2)
{
    digitalWrite(PIN_V1, v1 ? HIGH : LOW);
    digitalWrite(PIN_V2, v2 ? HIGH : LOW);
}

void valves_alarm(bool on)
{
    digitalWrite(PIN_ALARM, on ? HIGH : LOW);
}