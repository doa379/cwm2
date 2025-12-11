#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct {
  int pix;
  XRenderColor xr;
  XftColor xft;
} clr_t;

int clr_init(void);
void clr_deinit(void);
