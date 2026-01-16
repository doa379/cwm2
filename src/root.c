#include <X11/Xlib.h>
#include <stdio.h>

#include "root.h"
#include "input.h"
#include "font.h"
#include "cblk.h"

extern Display* dpy;
extern font_t font;
extern int rootbg;

static cblk_t cblk;
static size_t const NRES = 100;

int
root_init(void) {
  XSetWindowAttributes wa = {
    .cursor = font.crs.ptr,
    .event_mask =
      SubstructureRedirectMask |
      SubstructureNotifyMask |
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

void
root_deinit(void) {
  XClearWindow(dpy, DefaultRootWindow(dpy));
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
      CurrentTime);
}

int
root_wins_init(void) {
  cblk = cblk_init(sizeof(Window), NRES);
  if (cblk.blk == NULL) {
    fprintf(stderr, "Failed to alloc root wins\n");
    return -1;
  }

  return 0;
}

void
root_wins_deinit(void) {
  cblk_deinit(&cblk);
}

void
root_query(void) {
  Window root = DefaultRootWindow(dpy);
  Window parent;
  Window* wins;
  unsigned n;
  if (XQueryTree(dpy, root, &root, &parent, &wins, &n)) {
    cblk_clear(&cblk);
    for (unsigned i = 0; i < n; i++)
      cblk_map(&cblk, &wins[i]);
  
    XFree(wins);
  }
}

void root_ev_enqueue(void) {
  if (cblk.size == 0)
    return;

  XWindowAttributes wa;
  Window* win = cblk.front;
  do {
    if (XGetWindowAttributes(dpy, *win, &wa) &&
        wa.map_state == IsViewable) {
      XEvent xev = {
        .xmaprequest = (XMapRequestEvent) {
          .type = MapRequest,
          .display = dpy,
          .send_event = False,
          .parent = DefaultRootWindow(dpy),
          .window = *win
        }
      };

      XPutBackEvent(dpy, &xev);
    }
    
    win = cblk_next(&cblk, win);
  } while (win != cblk.front);
}

int
root_stack_dist(Window query) {
  if (cblk.size == 0)
    return -1;

  Window const* win = cblk_val_find(&cblk, &query);
  return win ? cblk_dist(&cblk, win) : -1;
}

Window
root_ptr_query(int* const x_root, int* const y_root) {
  Window root;
  Window child;
  int x_win;
  int y_win;
  unsigned mask;
  return XQueryPointer(dpy, DefaultRootWindow(dpy), &root, 
    &child, x_root, y_root, &x_win, &y_win, &mask) ? 
      child : None;
}
