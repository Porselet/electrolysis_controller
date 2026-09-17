#include <Arduino.h>
#include <EEPROM.h>
#include "settings.h"
#include "config.h"

float g_fill_threshold = FILL_THRESHOLD;
uint32_t g_hold_time_sec = HOLD_TIME_SEC;

static bool dirty = false;
static uint32_t dirty_timer_ms = 0;

// ---------------------------------------------------------------------------
// Чтение
// ---------------------------------------------------------------------------
void settings_load(void)
{
    uint16_t magic = 0;
    EEPROM.get(EEPROM_ADDR_MAGIC, magic);

    if (magic != EEPROM_MAGIC)
    {
        // EEPROM пустая или от другой версии — используем дефолты из config.h
        g_fill_threshold = FILL_THRESHOLD;
        g_hold_time_sec = HOLD_TIME_SEC;
        return;
    }

    EEPROM.get(EEPROM_ADDR_FILL, g_fill_threshold);
    EEPROM.get(EEPROM_ADDR_HOLD, g_hold_time_sec);

    // Санитарная проверка: если мусор — возвращаем дефолты
    if (g_fill_threshold < FILL_MIN_ATM || g_fill_threshold > FILL_MAX_ATM)
    {
        g_fill_threshold = FILL_THRESHOLD;
    }
    uint32_t max_sec = (uint32_t)HOLD_MAX_MIN * 60UL;
    if (g_hold_time_sec > max_sec)
    {
        g_hold_time_sec = HOLD_TIME_SEC;
    }
}

// ---------------------------------------------------------------------------
// Запись
// ---------------------------------------------------------------------------
static void write_all(void)
{
    uint16_t magic = EEPROM_MAGIC;
    EEPROM.put(EEPROM_ADDR_MAGIC, magic);
    EEPROM.put(EEPROM_ADDR_FILL, g_fill_threshold);
    EEPROM.put(EEPROM_ADDR_HOLD, g_hold_time_sec);
    dirty = false;
    dirty_timer_ms = 0;
}

void settings_mark_dirty(void)
{
    dirty = true;
    dirty_timer_ms = 0;
}

void settings_tick(uint32_t dt_ms)
{
    if (!dirty)
        return;
    dirty_timer_ms += dt_ms;
    if (dirty_timer_ms >= 10000UL)
    {
        settings_save_now();
    }
}

void settings_save_now(void)
{
    if (!dirty)
        return;

    // Сравниваем с тем, что уже лежит в EEPROM. Если значения
    // не изменились — не пишем, экономим циклы ячейки EEPROM.
    float fill_old = 0.0f;
    uint32_t hold_old = 0;
    EEPROM.get(EEPROM_ADDR_FILL, fill_old);
    EEPROM.get(EEPROM_ADDR_HOLD, hold_old);

    if (fill_old == g_fill_threshold && hold_old == g_hold_time_sec)
    {
        dirty = false;
        dirty_timer_ms = 0;
        return;
    }

    write_all();
}