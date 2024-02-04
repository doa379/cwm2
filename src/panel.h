#pragma once

#include <x.h>

void init_panel(const X_t*, const unsigned);
void deinit_panel(const X_t*);
GC init_gc(const X_t*);
void deinit_gc(const GC, const X_t*);
void draw_root(const X_t*, const char*, const size_t, const int, const int);
