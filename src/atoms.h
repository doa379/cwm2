#pragma once

#include <X11/Xlib.h>

enum prop {
  WM_PROTOCOLS,
  WM_NAME,
  WM_DELETE_WINDOW,
  WM_STATE,
  WM_TAKE_FOCUS,
  NET_SUPPORTED,
  NET_WM_STATE,
  NET_WM_NAME,
  NET_WM_WINDOW_OPACITY,
  NET_ACTIVE_WINDOW,
  NET_WM_STATE_FULLSCREEN,
  NET_WM_WINDOW_TYPE,
  NET_WM_WINDOW_TYPE_DIALOG,
  NET_CLIENT_LIST,
  NET_NUMBER_OF_DESKTOPS,
  NET_WM_DESKTOP,
  NET_CURRENT_DESKTOP,
  NET_SHOWING_DESKTOP,
  NPROPS
};

void init_atoms(Display*);
Atom atom(const enum prop);
void addprop_clientlist(const Window);
void clrprop_clientlist();
void delprop_actwindow(const Window);
void setprop_nwks(const int);
void setprop_wks(const int);
