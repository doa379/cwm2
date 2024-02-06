#pragma once

#include <X11/Xlib.h>

static const int ROOTMASK = { 
  SubstructureRedirectMask | 
  SubstructureNotifyMask | 
  ButtonPressMask |
  PointerMotionMask |
  EnterWindowMask |
  LeaveWindowMask |
  StructureNotifyMask |
  PropertyChangeMask
};

bool init_clients();
void deinit_clients();
void init_atoms(Display*);
void init_windows(Display*, const Window);
