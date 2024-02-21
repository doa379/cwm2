#pragma once

#include <X11/Xlib.h>

void init_panel(Display*, const Window);
void deinit_panel(Display*);
GC init_gc(Display*, const Window);
void deinit_gc(Display*, const GC);
void draw_root(Display*, const Window, const char*);
