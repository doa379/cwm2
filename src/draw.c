#include <X11/Xlib.h>
#include <string.h>

static Display* dpy;
static Window rootw;
static Drawable drawable;
static const char* FONT = { 
  // These are a dependency for X11 so no checks necessary here
  //"-misc-fixed-medium-r-normal--0-0-100-100-c-0-iso10646-1" };
  "9x15bold" };
static XFontStruct* fn;
static unsigned bh;

unsigned dpywidth() {
  return DisplayWidth(dpy, DefaultScreen(dpy));
}

unsigned dpyheight() {
  return DisplayHeight(dpy, DefaultScreen(dpy));
}

unsigned dpydepth() {
  return DefaultDepth(dpy, DefaultScreen(dpy));
}

void init_draw(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
  drawable = XCreatePixmap(dpy, rootw, dpywidth(), dpyheight(), dpydepth());
  XSetWindowBackground(dpy, rootw, 0x006e99);
  XClearWindow(dpy, rootw);
  fn = XLoadQueryFont(dpy, FONT);
  bh = fn->ascent + fn->descent;
}

void deinit_draw() {
  XFreePixmap(dpy, drawable); 
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

Window init_shadow(const unsigned WIDTH, const unsigned HEIGHT) {
  return XCreateSimpleWindow(dpy, rootw, 0, 0, WIDTH, HEIGHT, 0, 0, 0x141414);
}

void draw_element(const GC GC, const size_t FG, const size_t BG, 
  const unsigned X0, const unsigned Y0, const unsigned X1, const unsigned Y1) {
  XSetForeground(dpy, GC, BG);
  XFillRectangle(dpy, rootw, GC, X0, Y0, X1, Y1);
}

void draw_wks(const char* S, const GC GC, const size_t FG, const size_t BG,
  const unsigned H_PX, unsigned* offset) {
  XClearWindow(dpy, rootw);
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(GC, FG, BG, 0, H_PX - bh, SW, H_PX - bh);
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, rootw, GC, HPAD_PX, H_PX - fn->descent, S, SLEN);
  *offset = SW;
}

void draw_status(const char* S, const GC GC, const size_t FG, const size_t BG,
  const unsigned W_PX, const unsigned H_PX, unsigned* offset) {
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(GC, FG, BG, *offset, H_PX - bh, W_PX - *offset, H_PX);
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, rootw, GC, *offset + HPAD_PX, H_PX - fn->descent, S, SLEN);
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
