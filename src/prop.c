#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>

#include "prop.h"

extern Display* dpy;
static char BUF[128];
static size_t BUFLEN = sizeof BUF - 1;

prop_t prop;

void
prop_init(void) {
  prop = (prop_t) {
    .utf8string = XInternAtom(dpy, "UTF8_STRING", False),
    .wm_proto = XInternAtom(dpy, "WM_PROTOCOLS", False),
    .wm_state = XInternAtom(dpy, "WM_STATE", False),
    .wm_delwin = 
      XInternAtom(dpy, "WM_DELETE_WINDOW", False),
    .wm_iconame = XInternAtom(dpy, "WM_ICON_NAME", False),
    .net_supported = 
      XInternAtom(dpy, "_NET_SUPPORTED", False),
    .net_check = 
      XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False),
    .net_name = XInternAtom(dpy, "_NET_WM_NAME", False),
    .net_iconame = 
      XInternAtom(dpy, "_NET_WM_ICON_NAME", False),
    .net_clients = 
      XInternAtom(dpy, "_NET_CLIENT_LIST", False),
    .net_state = XInternAtom(dpy, "_NET_WM_STATE", False),
    .net_type = 
      XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False),
    .net_fs = 
      XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False),
    .net_actwin = 
      XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False),
  };
}

static int 
prop_text(Window const win, Atom const atom) {
  BUF[0] = '\0';
  XTextProperty prop;
  if (XGetTextProperty(dpy, win, &prop, atom) == 0 || 
      prop.nitems == 0) {
    return -1;
  }
    
  if (prop.encoding == XA_STRING) {
    strncpy(BUF, (char*) prop.value, BUFLEN);
  } else {
    char** list = NULL;
    int n = 0;
    if (XmbTextPropertyToTextList(dpy, &prop, &list, &n) ==
        Success && n > 0 && *list) {
      strncpy(BUF, *list, BUFLEN);
      XFreeStringList(list);
    }
  }

  BUF[BUFLEN] = '\0';
  XFree(prop.value);
  return 0;
}

char const*
prop_root(void) {
  return prop_name(DefaultRootWindow(dpy));
}
 
char const*
prop_name(Window const win) {
  if (prop_text(win, prop.net_name) != 0) {
 	  prop_text(win, XA_WM_NAME);
  }

  return BUF;
}

char const*
prop_ico(Window const win) {
  if (prop_text(win, prop.net_iconame) != 0 &&
    prop_text(win, prop.wm_iconame) != 0) {
      return prop_name(win);
  }

  return BUF;
}

static void
prop_win_msg(Window const win, Atom const type, 
Atom const msg) {
  XEvent ev = { 0 };
  ev.type = ClientMessage;
  ev.xclient.window = win;
  ev.xclient.message_type = type;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = msg;
  ev.xclient.data.l[1] = CurrentTime;
  XSendEvent(dpy, win, False, NoEventMask, &ev);
  XSync(dpy, False);
}

void
prop_win_del(Window const win) {
  prop_win_msg(win, prop.wm_proto, prop.wm_delwin);
}

void
prop_win_fs(Window const win) {
  XEvent ev = { 0 };
  ev.type = ClientMessage;
  ev.xclient.window = win;
  ev.xclient.message_type = prop.net_state;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
  ev.xclient.data.l[1] = prop.net_fs;
  ev.xclient.data.l[2] = 0;
  XSendEvent(dpy, DefaultRootWindow(dpy), False, 
    SubstructureNotifyMask | SubstructureRedirectMask, &ev);
  XSync(dpy, False);
}

void
prop_win_config(Window const win, int const x, int const y,
int const w, int const h, int const bw) {
  XConfigureEvent ce = { 0 };
  ce.type = ConfigureNotify;
  ce.display = dpy;
  ce.event = win;
  ce.window = win;
  ce.x = x;
  ce.y = y;
  ce.width  = w;
  ce.height = h;
  ce.border_width = bw;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(dpy, win, False, StructureNotifyMask,
    (XEvent*) &ce);
  XSync(dpy, False);
}
 
char const*
prop_atom_name(Atom const atom) {
  char* atom_name = XGetAtomName(dpy, atom);
  if (atom_name) {
    strncpy(BUF, atom_name, BUFLEN);
    BUF[BUFLEN] = '\0';
    XFree(atom_name);
  }

  return BUF;
}

void
prop_state_set(Window const win, long const state) {
  long const data[2] = { state, None };
  XChangeProperty(dpy, win, prop.wm_state, prop.wm_state,
    32, PropModeReplace, (unsigned char*) data, 2);
}
