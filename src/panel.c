#include <string.h>
#include <panel.h>
#include <util.h>
#include <../config.h>

static Drawable drawable;
static GC rootgc;
static pair_t dpysize;

void init_panel(Display* dpy, const Window ROOTW) {
  const int SCRN = DefaultScreen(dpy);
  dpysize = (pair_t) { DisplayWidth(dpy, SCRN), DisplayHeight(dpy, SCRN) };
  drawable = XCreatePixmap(dpy, ROOTW, dpysize.x, dpysize.y, 
    DefaultDepth(dpy, SCRN));
  rootgc = init_gc(dpy, ROOTW);
}

void deinit_panel(Display* dpy) {
  XFreeGC(dpy, rootgc);
  XFreePixmap(dpy, drawable); 
}

GC init_gc(Display* dpy, const Window ROOTW) {
  GC gc = XCreateGC(dpy, ROOTW, 0, NULL);
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(Display* dpy, const GC GC) {
  XFreeGC(dpy, GC);
}

void draw_root(Display* dpy, const Window ROOTW, const char* S) {
  XSetForeground(dpy, rootgc, TITLEBG);
  XFillRectangle(dpy, ROOTW, rootgc, 0, 0, dpysize.x, BARH);
  XSetForeground(dpy, rootgc, TITLEFG);
  XDrawString(dpy, ROOTW, rootgc, 0, BARH - 2, S, strlen(S));
}
