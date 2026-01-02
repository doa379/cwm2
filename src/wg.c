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
wg_init(Window const parwin, int const x, int const y, 
  int const w, int const h, int const bdrw) {
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
    .x1 = x + w + 2 * bdrw,
    .y1 = y + h + 2 * bdrw,
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
  if (wg->pixmap)
    XFreePixmap(dpy, wg->pixmap);

  XDestroyWindow(dpy, wg->win);
}

void
wg_str_set(wg_t* const wg, wchar_t const* str) {
  wcsncpy(wg->str.data, str, sizeof wg->str.data - 1);
  wg->str.len = wcslen(str);
  if (wg->str.len) {
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
    int const ty = 0.5 * 
      (wg->h - 2 * wg->bdrw + font.scent);
    XftDrawStringUtf8(wg->xft, &clr_pair[clr].fg.xft, 
        font.xft, tx, ty, 
          (XftChar8*) wg->str.data, wg->str.len);
  }
}

void
wg_win_bgset(Window const win, unsigned const clr) {
  XSetWindowBackground(dpy, win, clr_pair[clr].bg.pix);
  XClearWindow(dpy, win);
}

void
wg_win_bdrset(Window const win, unsigned const clr) {
  XSetWindowBorder(dpy, win, clr_pair[clr].bg.pix);
}

void
wg_gc_bgset(GC const gc, unsigned const clr) {
  XSetBackground(dpy, gc, clr_pair[clr].bg.pix);
  XSetForeground(dpy, gc, clr_pair[clr].fg.pix);
}

void
wg_pixmap_fill(wg_t const* wg, unsigned const clr) {
  wg_gc_bgset(wg->gc, clr);
  XFillRectangle(dpy, wg->win, wg->gc, 
      wg->x, wg->y, wg->w, wg->h);
  XCopyPlane(dpy, wg->pixmap, wg->win, wg->gc, 
      0, 0, wg->w, wg->h, 0, 0, 1);
}

int
wg_win_move(wg_t* const wg, int const x, int const y) {
  if (XMoveWindow(dpy, wg->win, x, y)) {
    wg->x0 = wg->x;
    wg->x = x;
    wg->y0 = wg->y;
    wg->y = y;
    wg->x1 = wg->x + wg->w + 2 * wg->bdrw;
    wg->y1 = wg->y + wg->h + 2 * wg->bdrw;
    return 0;
  }

  return -1;
}

int
wg_win_resize(wg_t* const wg, int const w, int const h) {
  if (XResizeWindow(dpy, wg->win, w, h)) {
    wg->h0 = wg->h;
    wg->h = h;
    wg->w0 = wg->w;
    wg->w = w;
    wg->x1 = wg->x + wg->w + 2 * wg->bdrw;
    wg->y1 = wg->y + wg->h + 2 * wg->bdrw;
    return 0;
  }

  return -1;
}
