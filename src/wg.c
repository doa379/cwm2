#include <X11/Xutil.h>
#include <unistd.h>
#include <stddef.h>

#include "wg.h"
#include "font.h"
#include "clr.h"

extern Display* dpy;
extern font_t font;
extern clr_pair_t clr_pair[];

wg_t
wg_init(Window const parwin, int const w, int const h, 
  int const bdrw) {
  Window const win = XCreateSimpleWindow(dpy, parwin, 
      0, 0, w, h, bdrw, 0, 0);
  XMapRaised(dpy, win);
  XftDraw* const xft = XftDrawCreate(dpy, win, 
    DefaultVisual(dpy, DefaultScreen(dpy)),
      DefaultColormap(dpy, DefaultScreen(dpy)));
  wg_t wg = {
    .win = win,
    .w = w,
    .h = h,
    .bdrw = bdrw,
    .pixmap = 0,
    .gc = XCreateGC(dpy, win, 0, NULL),
    .xft = xft,
  };

  return wg;
}

void
wg_deinit(wg_t* const wg) {
  XftDrawDestroy(wg->xft);
  XFreeGC(dpy, wg->gc);
  if (wg->pixmap) {
    XFreePixmap(dpy, wg->pixmap);
  }

  XDestroyWindow(dpy, wg->win);
}

void
wg_str_set(wg_t* const wg, char const* str) {
  size_t const len = strlen(str);
  if (len) {
    wg->str.len = len > sizeof wg->str.data ? 
      sizeof wg->str.data - 1 : len;
    strncpy(wg->str.data, str, wg->str.len);
    XGlyphInfo extents;
    XftTextExtentsUtf8(dpy, font.xft, 
      (XftChar8*) wg->str.data, wg->str.len, &extents);
    wg->str.ext = extents.xOff;
  }
}

void
wg_str_draw(wg_t* const wg, unsigned const clr, 
    unsigned const tx) {
  if (wg->str.len) {
    wg_win_bgclr(wg->win, clr);
    int const ty = 0.5 * 
      (wg->h - 2 * wg->bdrw + font.scent);
    XftDrawStringUtf8(wg->xft, &clr_pair[clr].fg.xft, 
      font.xft, tx, ty, (XftChar8*) wg->str.data, 
        wg->str.len);
  }
}

void
wg_win_bgclr(Window const win, unsigned const clr) {
  XSetWindowBackground(dpy, win, clr_pair[clr].bg.pix);
  XClearWindow(dpy, win);
}

void
wg_win_bdrclr(Window const win, unsigned const clr) {
  XSetWindowBorder(dpy, win, clr_pair[clr].bg.pix);
}
  
void
wg_bdrw(wg_t* const wg, int const w) {
  if (XSetWindowBorderWidth(dpy, wg->win, w)) {
    wg->bdrw = w;
  }
}

void
wg_gc_bgfgclr(GC const gc, unsigned const clr) {
  XSetBackground(dpy, gc, clr_pair[clr].bg.pix);
  XSetForeground(dpy, gc, clr_pair[clr].fg.pix);
}

void
wg_gc_bgclr(GC const gc, unsigned const clr) {
  XSetBackground(dpy, gc, clr_pair[clr].bg.pix);
}

void
wg_gc_fgclr(GC const gc, unsigned const clr) {
  XSetForeground(dpy, gc, clr_pair[clr].fg.pix);
}

void
wg_pixmap_fill(wg_t const* wg, unsigned const clr) {
  wg_gc_bgfgclr(wg->gc, clr);
  XFillRectangle(dpy, wg->win, wg->gc, 
      0, 0, wg->w, wg->h);
  XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc, 
      0, 0, wg->w, wg->h, 0, 0, 1);
}

void
wg_win_resize(wg_t* const wg, int const w, int const h) {
  if (XResizeWindow(dpy, wg->win, w, h)) {
    wg->w = w;
    wg->h = h;
  }
}
