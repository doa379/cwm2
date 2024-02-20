#pragma once

#include <X11/Xlib.h>

void init_panel(Display*, const unsigned);
void deinit_panel(Display*);
GC init_gc(Display*);
void deinit_gc(Display*, const GC);
void draw_root(Display*, const char*, const size_t, const int, const int);
