#pragma once

#include <X11/Xlib.h>

void init_draw(Display*);
void deinit_draw();
GC init_gc();
void deinit_gc(const GC);
void refresh_rootw(const unsigned, const unsigned, const unsigned, 
  const unsigned);

void draw_element(const char*, const GC, const size_t, const size_t,
  unsigned *, const unsigned, const unsigned);
Pixmap create_pixmap(char*, const unsigned, const unsigned, size_t, 
  size_t);
void draw_icon(const Window);
void cascade(int*, int*, const unsigned, const unsigned, const unsigned,
  const unsigned);
