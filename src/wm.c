#include <wm.h>
#include <../config.h>

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

static atom_t atom;

void init_atoms(Display* dpy) {
  atom_t atom_ = {
    XInternAtom(dpy, "WM_PROTOCOLS", false),
    XInternAtom(dpy, "WM_NAME", false),
    XInternAtom(dpy, "WM_DELETE_WINDOW", false),
    XInternAtom(dpy, "WM_STATE", false),
    XInternAtom(dpy, "WM_TAKE_FOCUS", false),
    XInternAtom(dpy, "_NET_SUPPORTED", false),
    XInternAtom(dpy, "_NET_WM_STATE", false),
    XInternAtom(dpy, "_NET_WM_NAME", false),
    XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false),
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false),
    XInternAtom(dpy, "_NET_CLIENT_LIST", false),
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false),
    XInternAtom(dpy, "_NET_WM_DESKTOP", false),
    XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false),
    XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false)
  };

  atom = atom_;
}

void init_clients(Display* dpy, const Window root) {
  unsigned n;
  Window rootw, parw;
  Window* w;
  if (XQueryTree(dpy, root, &rootw, &parw, &w, &n)) {
    for (int i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && 
          wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = root;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, root, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
}

