#pragma once

#include <X11/Xlib.h>
#include <util.h>

pair_t init_window(Display*, const Window);
void append_window(Display*, const Window, const Window, const Atom);
int modmask(Display*);
