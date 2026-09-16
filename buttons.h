#ifndef BUTTONS_H
#define BUTTONS_H

enum Button {
    BTN_NONE = 0,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_SELECT
};

void buttons_init(void);
enum Button buttons_poll(void);

#endif