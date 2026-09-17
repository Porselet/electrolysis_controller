#include <math.h>
#include "safety.h"
#include "config.h"

// ---------------------------------------------------------------------------
// Проверка: валиден ли датчик
// ---------------------------------------------------------------------------
static bool is_valid(const struct SensorReading *r)
{
    return r->state == SENS_OK;
}

// ---------------------------------------------------------------------------
// Основная проверка
// ---------------------------------------------------------------------------
void safety_check(const struct SensorReading *p1,
                  const struct SensorReading *p2,
                  struct SafetyResult *result)
{

    bool p1_ok = is_valid(p1);
    bool p2_ok = is_valid(p2);

    result->fault = FAULT_NONE;
    result->degraded = DEG_NONE;
    result->P = 0.0f;
    result->dP = 0.0f;
    result->pressure_valid = false;

    // Оба невалидны — авария
    if (!p1_ok && !p2_ok)
    {
        result->fault = FAULT_SENSOR;
        return;
    }

    // Один валиден — деградация
    if (p1_ok && !p2_ok)
    {
        result->degraded = DEG_P1;
        result->P = p1->bar;
        result->pressure_valid = true;
    }
    else if (!p1_ok && p2_ok)
    {
        result->degraded = DEG_P2;
        result->P = p2->bar;
        result->pressure_valid = true;
    }
    // Оба валидны — проверяем расхождение
    else
    {
        float d = p1->bar - p2->bar;
        if (d < 0.0f)
            d = -d;
        result->dP = d;

        if (d > P_DIFF_MAX)
        {
            result->fault = FAULT_MISMATCH;
            return;
        }

        result->P = (p1->bar + p2->bar) * 0.5f;
        result->pressure_valid = true;
    }

    // Overpressure по любому валидному датчику
    if (p1_ok && p1->bar > P_MAX_BAR)
    {
        result->fault = FAULT_OVER;
        return;
    }
    if (p2_ok && p2->bar > P_MAX_BAR)
    {
        result->fault = FAULT_OVER;
        return;
    }
}

// ---------------------------------------------------------------------------
// Безопасное состояние при аварии
// ---------------------------------------------------------------------------
void safety_apply_fault(enum FaultReason reason, bool v2_current,
                        bool *v1_out, bool *v2_out)
{

    // V1 всегда закрыт при аварии
    *v1_out = false;

    switch (reason)
    {
    case FAULT_OVER:
        // Избыток давления — стравить
        *v2_out = true;
        break;

    case FAULT_SENSOR:
    case FAULT_MISMATCH:
        // Не знаем состояние — оставить V2 как был
        *v2_out = v2_current;
        break;

    default:
        *v2_out = false;
        break;
    }
}