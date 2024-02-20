#include <panel.h>

static int scrn;
static Window rootw;
static Drawable drawable;
static GC rootgc;
static unsigned H;

void init_panel(Display* dpy, const unsigned BARH) {
  scrn = DefaultScreen(dpy);
  rootw = XRootWindow(dpy, scrn);
  drawable = XCreatePixmap(dpy, rootw, DisplayWidth(dpy, scrn), 
    DisplayHeight(dpy, scrn), DefaultDepth(dpy, scrn));
  rootgc = init_gc(dpy);
  H = BARH;
}

void deinit_panel(Display* dpy) {
  XFreeGC(dpy, rootgc);
  XFreePixmap(dpy, drawable); 
}

GC init_gc(Display* dpy) {
  GC gc = XCreateGC(dpy, rootw, 0, NULL);
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(Display* dpy, const GC GC) {
  XFreeGC(dpy, GC);
}

void draw_root(Display* dpy, const char* S, const size_t LEN,
const int FG, const int BG) {
  XSetForeground(dpy, rootgc, BG);
  XFillRectangle(dpy, rootw, rootgc, 0, 0, DisplayWidth(dpy, scrn), H);
  XSetForeground(dpy, rootgc, FG);
  XDrawString(dpy, rootw, rootgc, 0, H - 2, S, LEN);
}
