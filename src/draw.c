#include <X11/Xlib.h>
#include <string.h>
#include <draw.h>

static Display* dpy;
static Window rootw;
static const char* FONT = { 
  // These are a dependency for X11 so no checks necessary here
  //"-misc-fixed-medium-r-normal--0-0-100-100-c-0-iso10646-1" };
  "9x15bold" };
static XFontStruct* fn;
static unsigned bh;
static Drawable drawable;
static GC statusgc;
static GC wksgc;
static GC rootgc;

void init_draw(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
  fn = XLoadQueryFont(dpy, FONT);
  bh = fn->ascent + fn->descent;
  
  drawable = XCreatePixmap(dpy, rootw,
    // Drawable over full extent of mon estate
    DisplayWidth(dpy, DefaultScreen(dpy)),
    DisplayHeight(dpy, DefaultScreen(dpy)),
    DefaultDepth(dpy, DefaultScreen(dpy)));
  
  statusgc = init_gc();
  wksgc = init_gc();
  rootgc = XCreateGC(dpy, rootw, 0, NULL);
  XSetForeground(dpy, rootgc, 0xffffff);
  XFillRectangle(dpy, drawable, rootgc, 0, 0, 
    DisplayWidth(dpy, DefaultScreen(dpy)),
    DisplayHeight(dpy, DefaultScreen(dpy)));

  const unsigned D = { 5 };
  XSetForeground(dpy, rootgc, 0x1f1f1f);
  for (int i = { 0 };
      i < DisplayWidth(dpy, DefaultScreen(dpy)); i++)
    for (int j = { 0 }; 
      j < DisplayHeight(dpy, DefaultScreen(dpy)); j++) {
      const unsigned X = { D * i };
      const unsigned Y = { D * j };
      XFillArc(dpy, drawable, rootgc, X - 2, Y - 2, 5, 5, 0, 360 * 64);
    }
}

void deinit_draw() {
  XFreePixmap(dpy, drawable);
  XFreeGC(dpy, rootgc);
  XFreeGC(dpy, wksgc);
  XFreeGC(dpy, statusgc);
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

void draw_element(const GC GC, const size_t FG, const size_t BG, 
  const unsigned X0, const unsigned Y0, const unsigned X1, const unsigned Y1) {
  XSetForeground(dpy, GC, BG);
  XFillRectangle(dpy, rootw, GC, X0, Y0, X1, Y1);
}

void draw_wks(const char* S, const size_t FG, const size_t BG,
  const unsigned H, unsigned* offset) {
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(wksgc, FG, BG, 0, H - bh, SW, H - bh);
  XSetForeground(dpy, wksgc, FG);
  XDrawString(dpy, rootw, wksgc, HPAD_PX, H - fn->descent, S, SLEN);
  *offset = SW;
}

void draw_status(const char* S, const size_t FG, const size_t BG,
  const unsigned W, const unsigned H, unsigned* offset) {
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(statusgc, FG, BG, *offset, H - bh, W - *offset, H);
  XSetForeground(dpy, statusgc, FG);
  XDrawString(dpy, rootw, statusgc, *offset + HPAD_PX, H - fn->descent, S, SLEN);
  *offset = SW;
}

void draw_client(const char* S, const GC GC, const size_t FG, const size_t BG,
  unsigned* offset) {
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(GC, FG, BG, *offset, 0, SW, bh);
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, rootw, GC, *offset + HPAD_PX, fn->ascent, S, SLEN);
  *offset += SW;
}

void cascade(int* x, int* y, const unsigned W, const unsigned H, 
  const unsigned WEX, const unsigned HEX) {
  // Args:  W, H of window,
  //        Width Extent WEX, Height Extent HEX of mon estate
  static short grav;
  const char DIRX = { grav >> 0 & 1 ? -1 : 1 };
  const char DIRY = { grav >> 1 & 1 ? -1 : 1 };
  static const unsigned SH = { 14 };
  *x += *y != 0 ? DIRX * bh : 0; 
  *y += DIRY * bh;
  if (*x + W > WEX) {
    *x = WEX - W;
    grav |= 1 << 0;
  } if (*y + H > HEX - bh - SH) {
    *y = HEX - H - bh - SH;
    grav |= 1 << 1; 
  } if (*x < 0) {
    *x = 0;
    grav ^= 1 << 0;
  } if (*y < bh) {
    *y = bh;
    grav ^= 1 << 1;
  }
}
