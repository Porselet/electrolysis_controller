#include <Arduino.h>
#include "sensors.h"

// ---------------------------------------------------------------------------
// Медиана из N отсчётов АЦП
// ---------------------------------------------------------------------------
static int cmp_int(const void *a, const void *b)
{
    int va = *(const int *)a;
    int vb = *(const int *)b;
    return (va > vb) - (va < vb);
}

static int read_adc_median(uint8_t pin)
{
    int buf[MEDIAN_N];
    for (uint8_t i = 0; i < MEDIAN_N; i++)
    {
        buf[i] = analogRead(pin);
        delayMicroseconds(200);
    }
    qsort(buf, MEDIAN_N, sizeof(int), cmp_int);
    return buf[MEDIAN_N / 2];
}

// ---------------------------------------------------------------------------
// Одно показание
// ---------------------------------------------------------------------------
static void read_one(uint8_t pin, struct SensorReading *r)
{
    int raw = read_adc_median(pin);
    float v = raw * VREF / ADC_MAX;

    // Грубая проверка рельс до перевода в мА
    if (v < V_RAIL_LOW)
    {
        r->state = SENS_RAIL_LOW;
        r->bar = 0.0f;
        r->mA = 0.0f;
        return;
    }
    if (v > V_RAIL_HIGH)
    {
        r->state = SENS_RAIL_HIGH;
        r->bar = 0.0f;
        r->mA = 100.0f;
        return;
    }

    r->mA = (v / R_SHUNT) * 1000.0f;

    if (r->mA < MA_OPEN_MAX)
    {
        r->state = SENS_OPEN;
        r->bar = 0.0f;
        return;
    }
    if (r->mA > MA_SHORT_MIN)
    {
        r->state = SENS_SHORT;
        r->bar = 0.0f;
        return;
    }

    r->state = SENS_OK;
    r->bar = (r->mA - 4.0f) * P_RANGE / 16.0f;
    if (r->bar < 0.0f)
        r->bar = 0.0f;
}

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------
void sensors_init(void)
{
    // analogRead не требует pinMode
}

void sensors_read(struct SensorReading *p1, struct SensorReading *p2)
{
    read_one(PIN_P1, p1);
    read_one(PIN_P2, p2);
}