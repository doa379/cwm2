#pragma once

#include <X11/Xft/Xft.h>

typedef struct {
  Cursor ptr;
  Cursor move;
  Cursor resize;
  Cursor h;
  Cursor v;
} font_crs_t;

typedef struct {
  XftFont* xft;
  unsigned scent;
  /* Canonical width, height */
  unsigned ch;
  unsigned cw;
  font_crs_t crs;
} font_t;

int font_init(void);
void font_deinit(void);
