#pragma once

#include <X11/Xlib.h>

void status_init(Window const);
void status_deinit(void);
void status_draw(unsigned const);
void status_str_set(char const*);
void status_mon_str_set(char const*);
void status_mon_draw(unsigned const);
