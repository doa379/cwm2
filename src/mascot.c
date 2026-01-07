#include <X11/Xlib.h>

#include "palette.h"
#include "mon.h"
#include "wg.h"
#include "cblk.h"

#include "res/mascot.xbm"

extern Display* dpy;
extern cblk_t mons;
extern wg_t panel;
extern int rootbg;

static Pixmap pixmap;
static GC gc;

void
mascot_init(void) {
  pixmap = XCreateBitmapFromData(dpy, DefaultRootWindow(dpy), 
    (char const*) mascot_bits, mascot_width, mascot_height);
  gc = XCreateGC(dpy, DefaultRootWindow(dpy), 0, NULL);
}

void
mascot_deinit(void) {
  XFreeGC(dpy, gc);
  XFreePixmap(dpy, pixmap);
}

void
mascot_draw(void) {
  XSetBackground(dpy, gc, rootbg);
  XSetForeground(dpy, gc, palette_gray70);
  
  mon_t const* mon = mons.front;
  int const y = mon->h - mascot_height;
  XFillRectangle(dpy, DefaultRootWindow(dpy), gc, 
      0, y, mascot_width, mascot_height);
  XCopyPlane(dpy, pixmap, DefaultRootWindow(dpy), gc, 
      0, 0, mascot_width, mascot_height, 0, y, 1);
  XFlush(dpy);
}

