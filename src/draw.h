#pragma once

#include <X11/Xlib.h>

void init_draw(Display*);
void deinit_draw();
GC init_gc();
void deinit_gc(const GC);
void refresh_rootw(const unsigned, const unsigned, const unsigned, 
  const unsigned);
int vh();

unsigned string_len(const char*);
void fill_element(const GC, const size_t, const int, const int, const int,
  const int);
void draw_string(const char*, const Window, const GC, const size_t, const int,
  const int);
Pixmap create_pixmap(char*, const unsigned, const unsigned, size_t, size_t);
void draw_icon(const Window);
void cascade(int*, int*, const unsigned, const unsigned, const unsigned,
  const unsigned);
