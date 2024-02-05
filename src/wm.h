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

void init_atoms(Display*);
void init_clients(Display*, const Window);
