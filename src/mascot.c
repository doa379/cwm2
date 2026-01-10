#include <X11/Xlib.h>

#include "wg.h"
#include "cblk.h"
#include "res/mascot.xbm"

extern Display* dpy;

extern int COLORS[];

void
mascot_init(wg_t* const wg) {
  wg->pixmap = XCreateBitmapFromData(dpy, wg->win, 
    (char const*) mascot_bits, mascot_width, mascot_height);
  wg->gc = XCreateGC(dpy, wg->win, 0, NULL);
}

void
mascot_draw(wg_t* const wg, int const x, int const y) {
  XSetBackground(dpy, wg->gc, COLORS[wg_BG]);
  XSetForeground(dpy, wg->gc, COLORS[wg_ACT]);
  XFillRectangle(dpy, wg->win, wg->gc, x, y - mascot_height, 
    mascot_width, mascot_height);
  XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc, 0, 0, 
    mascot_width, mascot_height, x, y - mascot_height, 1);
  XFlush(dpy);
}

