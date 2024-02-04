#pragma once

#include <X11/Xlib.h>
#include <util.h>

typedef struct {
  Display* dpy;
  int scrn;
  Window root;
  pair_t dpysize;
  int modmask;
} X_t;
