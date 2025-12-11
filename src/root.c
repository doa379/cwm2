#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <stdio.h>

#include "input.h"
#include "cblk.h"

extern Display* dpy;

static cblk_t clis;

int root_init(void) {
  {
    XSetWindowAttributes wa = {
      .cursor = XCreateFontCursor(dpy, XC_left_ptr)
    };
  
    XChangeWindowAttributes(dpy, DefaultRootWindow(dpy),
      CWEventMask | CWCursor, &wa);
    XFreeCursor(dpy, wa.cursor);
  }

  XSelectInput(dpy, DefaultRootWindow(dpy),
    SubstructureRedirectMask |
    SubstructureNotifyMask |
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask |
    EnterWindowMask |
    LeaveWindowMask |
    StructureNotifyMask |
    PropertyChangeMask |
    ExposureMask);

  input_keys_grab(DefaultRootWindow(dpy));
  XDeleteProperty(dpy, DefaultRootWindow(dpy), 
    XInternAtom(dpy, "_NET_CLIENT_LIST", False));
  return 0;
}

void root_deinit(void) {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
      CurrentTime);
  XDeleteProperty(dpy, DefaultRootWindow(dpy), 
    XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False));
}

int root_query_init(void) {
  clis = cblk_init(sizeof(Window), 100);
  if (clis.beg == NULL)
    return -1;
  
  return 0;
}

void root_query_deinit(void) {
  cblk_deinit(&clis);
}

void root_query_(void) {
  /* Dummies */
  Window d0;
  Window d1;
  Window* wins = NULL;
  unsigned n = 0;
  XQueryTree(dpy, DefaultRootWindow(dpy), &d0, &d1, &wins,
      &n);
  for (unsigned i = 0; i < n; i++) {
    XWindowAttributes wa;
    if (XGetWindowAttributes(dpy, wins[i], &wa) &&
        wa.map_state == IsViewable)
      cblk_map(&clis, &wins[i]);
  }

  XFree(wins);
}

void root_send_maps(void) {
  for (Window* c = clis.beg; c != clis.end; c++) {
    XEvent xev = {
      .xmaprequest = (XMapRequestEvent) {
        .type = MapRequest,
        .display = dpy,
        .send_event = False,
        .parent = DefaultRootWindow(dpy),
        .window = *c
      }
    };

    XPutBackEvent(dpy, &xev);
  }
}

void root_query(void) {
  /* Dummies */
  Window d0;
  Window d1;
  Window* wins = NULL;
  unsigned n = 0;
  XQueryTree(dpy, DefaultRootWindow(dpy), &d0, &d1, &wins,
      &n);
  for (unsigned i = 0; i < n; i++) {
    XWindowAttributes wa;
    if (XGetWindowAttributes(dpy, wins[i], &wa) &&
        wa.map_state == IsViewable) {
      XEvent xev = {
        .xmaprequest = (XMapRequestEvent) {
          .type = MapRequest,
          .display = dpy,
          .send_event = False,
          .parent = DefaultRootWindow(dpy),
          .window = wins[i]
        }
      };

      XPutBackEvent(dpy, &xev);
    }
  }

  XFree(wins);
}
