#include <X11/Xutil.h>
#include <unistd.h>
#include <stddef.h>

#include "wg.h"
#include "clr.h"

extern Display* dpy;
extern XftFont* xftfont;
extern clr_t CLR[];

wg_t wg_init(Window const parwin, 
  int const x, int const y, int const w, int const h, 
  int const bdrw) {
  Window const win = XCreateSimpleWindow(dpy, parwin, 
      x, y, w, h, bdrw, 0, 0);
  XMapRaised(dpy, win);
  XftDraw* const xft = XftDrawCreate(dpy, win, 
    DefaultVisual(dpy, DefaultScreen(dpy)),
      DefaultColormap(dpy, DefaultScreen(dpy)));
  wg_t wg = {
    .win = win,
    .x0 = x,
    .x = x,
    .y0 = y,
    .y = y,
    .w0 = w,
    .w = w,
    .h0 = h,
    .h = h,
    .bdrw = bdrw,
    .pixmap = 0,
    .gc = XCreateGC(dpy, win, 0, NULL),
    .xft = xft,
  };

  return wg;
}

void wg_deinit(wg_t* const wg) {
  XftDrawDestroy(wg->xft);
  XFreeGC(dpy, wg->gc);
  if (wg->pixmap)
    XFreePixmap(dpy, wg->pixmap);

  XDestroyWindow(dpy, wg->win);
}

void wg_str_set(wg_t* const wg, char const* str) {
  strncpy(wg->str.data, str, sizeof wg->str.data - 1);
  wg->str.len = strlen(str);
  if (wg->str.len) {
    XGlyphInfo extents;
    XftTextExtentsUtf8(dpy, xftfont, 
      (XftChar8*) wg->str.data, wg->str.len, &extents);
    wg->str.ext = extents.xOff;
  }
}

void wg_str_draw(wg_t* const wg, unsigned const clr, 
    unsigned const tx) {
  if (wg->str.len) {
    int const ty = 0.5 * 
      (wg->h - 
       2 * wg->bdrw + xftfont->ascent - xftfont->descent);
    XftDrawStringUtf8(wg->xft, &CLR[clr + 3].xft, xftfont,
      tx, ty, (XftChar8*) wg->str.data, wg->str.len);
  }
}

void wg_win_setbg(Window const win, unsigned const clr) {
  XSetWindowBackground(dpy, win, CLR[clr].pix);
  XClearWindow(dpy, win);
}

void wg_win_setbdr(Window const win, unsigned const clr) {
  XSetWindowBorder(dpy, win, CLR[clr].pix);
}

void wg_gc_setbg(GC const gc, unsigned const clr) {
  XSetBackground(dpy, gc, CLR[clr].pix);
  XSetForeground(dpy, gc, CLR[clr + 3].pix);
}

void wg_pixmap_fill(wg_t const* wg, unsigned const clr) {
  wg_gc_setbg(wg->gc, clr);
  XFillRectangle(dpy, wg->win, wg->gc, 
      wg->x, wg->y, wg->w, wg->h);
  XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc, 
      0, 0, wg->w, wg->h, 0, 0, 1);
}
