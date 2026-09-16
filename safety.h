#ifndef SAFETY_H
#define SAFETY_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

// ---------------------------------------------------------------------------
// Причины аварии
// ---------------------------------------------------------------------------
enum FaultReason {
    FAULT_NONE = 0,
    FAULT_OVER,        // P > P_MAX
    FAULT_SENSOR,      // оба датчика невалидны
    FAULT_MISMATCH     // |P1 - P2| > P_DIFF_MAX
};

// ---------------------------------------------------------------------------
// Причины деградации
// ---------------------------------------------------------------------------
enum DegradedReason {
    DEG_NONE = 0,
    DEG_P1,            // валиден только P2
    DEG_P2             // валиден только P1
};

// ---------------------------------------------------------------------------
// Результат проверки безопасности
// ---------------------------------------------------------------------------
struct SafetyResult {
    enum FaultReason    fault;
    enum DegradedReason degraded;
    float               P;              // рабочее давление (валидно при !fault)
    float               dP;             // расхождение датчиков
    bool                pressure_valid; // false при fault
};

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------
void safety_check(const struct SensorReading* p1,
                  const struct SensorReading* p2,
                  struct SafetyResult* result);

void safety_apply_fault(enum FaultReason reason, bool v2_current,
                        bool* v1_out, bool* v2_out);

#endif