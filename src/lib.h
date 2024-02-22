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

typedef struct {
  void* blk;
  void* prev;
  void* curr;
  size_t unit;
  size_t reserve;
  size_t size;
} blk_t;

typedef struct {
  Atom PROTO;
  Atom NAME;
  Atom DELETE_WINDOW;
  Atom STATE;
  Atom TAKE_FOCUS;
  Atom SUPPORTED;
  Atom WM_STATE;
  Atom WM_NAME;
  Atom ACTIVE_WINDOW;
  Atom WM_STATE_FULLSCREEN;
  Atom WM_WINDOW_TYPE;
  Atom WM_WINDOW_TYPE_DIALOG;
  Atom CLIENT_LIST;
  Atom NUMBER_OF_DESKTOPS;
  Atom WM_DESKTOP;
  Atom CURRENT_DESKTOP;
  Atom SHOWING_DESKTOP;
} atom_t;

blk_t init_blk(const size_t, const size_t);
void deinit_blk(blk_t*);
void* init_dev(blk_t*, const void*);
void deinit_dev(blk_t*, const void*);
atom_t init_atoms(Display*);
int modmask(Display*);
void init_windows(Display*, const Window);
