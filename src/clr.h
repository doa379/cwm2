#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct {
  int pix;
  XRenderColor xr;
  XftColor xft;
} clr_t;

typedef struct {
  clr_t bg;
  clr_t fg;
} clr_pair_t;

int clr_init(void);
void clr_deinit(void);
