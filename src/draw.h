#pragma once

#include <X11/Xlib.h>

unsigned dpywidth();
unsigned dpyheight();
unsigned dpydepth();
void init_draw(Display*);
void deinit_draw();
GC init_gc();
void deinit_gc(const GC);
void draw_wks(const char*, const GC, const size_t, const size_t, 
  const unsigned, unsigned*);
void draw_status(const char*, const GC, const size_t, const size_t, 
  const unsigned, const unsigned, unsigned*);
void draw_client(const char*, const GC, const size_t, const size_t, unsigned*);
