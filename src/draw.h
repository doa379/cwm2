#pragma once

#include <X11/Xlib.h>

void init_draw(Display*);
void deinit_draw();
GC init_gc();
void deinit_gc(const GC);
void refresh_rootw(const unsigned, const unsigned, const unsigned, 
  const unsigned);
void draw_wks(const char*, const size_t, const size_t, const unsigned, 
  unsigned*);
void draw_status(const char*, const size_t, const size_t, const unsigned,
  const unsigned, unsigned*);
void draw_client(const char*, const GC, const size_t, const size_t, unsigned*);
void cascade(int*, int*, const unsigned, const unsigned, const unsigned,
  const unsigned);
