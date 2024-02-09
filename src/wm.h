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
void key(Display*, const Window, const int, const int);
void map(Display*, const Window, const Window);
static void quit(Display*, const Window);
static void kill(Display*, const Window);
