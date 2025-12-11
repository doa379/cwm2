#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>

#include "draw.h"
#include "palette.h"

static Display* dpy;
static Window rootw;
static const char* FONT = { 
  // These are a dependency for X11 so no checks necessary here
  //"-misc-fixed-medium-r-normal--0-0-100-100-c-0-iso10646-1" };
  "9x15bold" };
static XFontStruct* fn;
static int bh;
static int dpyw;
static int dpyh;
static int depth;
static Drawable drawable;
static GC rootgc;

void init_draw(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
  fn = XLoadQueryFont(dpy, FONT);
  bh = fn->ascent + fn->descent;
  dpyw = DisplayWidth(dpy, DefaultScreen(dpy));
  dpyh = DisplayHeight(dpy, DefaultScreen(dpy));
  depth = DefaultDepth(dpy, DefaultScreen(dpy));
  // Drawable over full extent of mon estate
  drawable = XCreatePixmap(dpy, rootw, dpyw, dpyh, depth);
  
  rootgc = XCreateGC(dpy, rootw, 0, NULL);
  // Allowable set custom wallpaper etc.
  XSetWindowBackground(dpy, rootw, Gray70);
  XClearWindow(dpy, rootw);
  /*
  XSetForeground(dpy, rootgc, Gray70);
  XFillRectangle(dpy, drawable, rootgc, 0, 0, dpyw, dpyh);
  const unsigned D = { 3 };
  XSetForeground(dpy, rootgc, Gray20);
  for (int i = { 0 }; i < dpyw; i++)
    for (int j = { 0 }; j < dpyh; j++)
      XFillArc(dpy, drawable, rootgc, D * i - 0, D * j - 0, 3, 3, 0, 360 * 64);
  refresh_rootw(0, 0, dpyw, dpyh);
  */
}

void deinit_draw() {
  XSetForeground(dpy, rootgc, Black);
  XFillRectangle(dpy, rootw, rootgc, 0, 0, dpyw, dpyh);
  XFreePixmap(dpy, drawable);
  XFreeGC(dpy, rootgc);
  XFreeFont(dpy, fn);
}

GC init_gc() {
  GC gc = { XCreateGC(dpy, rootw, 0, NULL) };
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(const GC GC) {
  XFreeGC(dpy, GC);
}

void refresh_rootw(const unsigned X0, const unsigned Y0, const unsigned X1,
  const unsigned Y1) {
  XCopyArea(dpy, drawable, rootw, rootgc, X0, Y0, X1, Y1, 0, 0);
}

int vh() {
  return bh;
}

unsigned string_len(const char* S) {
  const unsigned SLEN = { strlen(S) };
  return XTextWidth(fn, S, SLEN);
}

void fill_element(const GC GC, const size_t COL, const int X, const int Y, 
  const int W, const int H) {
  XSetForeground(dpy, GC, COL);
  XFillRectangle(dpy, rootw, GC, X, Y, W, H);
}

void draw_string(const char* S, const Window W, const GC GC, const size_t FG,
  const int X, const int Y) {
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, W, GC, X, Y - fn->descent, S, strlen(S));
}

void draw_element() {

}

Pixmap create_pixmap(char* data, const unsigned W, const unsigned H,
  size_t FG, size_t BG) {
  Pixmap ret = {
    XCreatePixmapFromBitmapData(dpy, drawable, data, W, H, FG, BG, depth)
  };
  XCopyArea(dpy, drawable, rootw, rootgc, 0, 0, 400, 300, 0, 0);
  return ret;
}

void cascade(int* x, int* y, const unsigned W, const unsigned H, 
  const unsigned WEX, const unsigned HEX) {
  // Args:  W, H of window,
  //        Width Extent WEX, Height Extent HEX of mon estate
  static short grav;
  const char DIRX = { grav >> 0 & 1 ? -1 : 1 };
  const char DIRY = { grav >> 1 & 1 ? -1 : 1 };
  //static const unsigned SH = { 14 };
  //*x += *y != 0 ? DIRX * bh : 0; 
  //*y += DIRY * bh;
  *x += *y != 0 ? *x * DIRX : 0; 
  *y += *y * DIRY;
  if (*x + W > WEX) {
    *x = WEX - W;
    grav |= 1 << 0;
  } if (*y + H > HEX) {
    *y = HEX - H;
    grav |= 1 << 1; 
  } if (*x < 0) {
    *x = 0;
    grav ^= 1 << 0;
  } if (*y < 0) {
    *y = 0;
    grav ^= 1 << 1;
  }
}
