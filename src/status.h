#pragma once

#include <X11/Xlib.h>
#include <wchar.h>

void status_init(Window const);
void status_deinit(void);
void status_focus(unsigned const);
void status_str_set(wchar_t const*);
