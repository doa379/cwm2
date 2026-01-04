#include <X11/Xlib.h>
#include <stdio.h>

#include "root.h"
#include "input.h"
#include "font.h"

extern Display* dpy;
extern font_t font;
extern int rootbg;

int root_init(void) {
  XSetWindowAttributes wa = {
    .cursor = font.crs.ptr,
    .event_mask = SubstructureRedirectMask |
      SubstructureNotifyMask |
      ButtonPressMask |
      PointerMotionMask |
      EnterWindowMask |
      LeaveWindowMask |
      StructureNotifyMask |
      PropertyChangeMask |
      ExposureMask,
  };

  XChangeWindowAttributes(dpy, DefaultRootWindow(dpy),
    CWEventMask | CWCursor, &wa);
  XSelectInput(dpy, DefaultRootWindow(dpy), 
      wa.event_mask);
  input_keys_grab(DefaultRootWindow(dpy));
  XSetWindowBackground(dpy, DefaultRootWindow(dpy), rootbg);
  XClearWindow(dpy, DefaultRootWindow(dpy));
  return 0;
}

void root_deinit(void) {
  XClearWindow(dpy, DefaultRootWindow(dpy));
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
      CurrentTime);
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

int
root_ptr(int* const x, int* const y) {
  int di;
  unsigned dui;
  Window dummy;
  return XQueryPointer(dpy, DefaultRootWindow(dpy), &dummy, 
    &dummy, x, y, &di, &di, &dui);
}
