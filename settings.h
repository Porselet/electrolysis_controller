#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

// Текущие значения уставок (загружены из EEPROM или дефолты из config.h)
extern float    g_fill_threshold;
extern uint32_t g_hold_time_sec;

// Загрузить из EEPROM. Если magic не совпал — берём дефолты из config.h.
void settings_load(void);

// Пометить, что есть несохранённые изменения.
void settings_mark_dirty(void);

// Периодический вызов: если dirty и прошло > 10 сек — записать.
void settings_tick(uint32_t dt_ms);

// Записать немедленно (например, при уходе с экрана уставок).
void settings_save_now(void);

#endif