#include <panel.h>

static Drawable drawable;
static GC root;
static int SCRN;
static unsigned H;

void init_panel(Display* dpy, const int SCRN_, const Window ROOT, 
const unsigned BARH) {
  drawable = XCreatePixmap(dpy, ROOT, DisplayWidth(dpy, SCRN_), 
    DisplayHeight(dpy, SCRN_), DefaultDepth(dpy, SCRN_));
  root = init_gc(dpy, ROOT);
  SCRN = SCRN_;
  H = BARH;
}

void deinit_panel(Display* dpy) {
  XFreeGC(dpy, root);
  XFreePixmap(dpy, drawable); 
}

GC init_gc(Display* dpy, const Window ROOT) {
  GC gc = XCreateGC(dpy, ROOT, 0, NULL);
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(Display* dpy, const GC GC) {
  XFreeGC(dpy, GC);
}

void draw_root(Display* dpy, const Window ROOT, const char* S, 
const size_t LEN, const int FG, const int BG) {
  XSetForeground(dpy, root, BG);
  XFillRectangle(dpy, ROOT, root, 0, 0, DisplayWidth(dpy, SCRN), H);
  XSetForeground(dpy, root, FG);
  XDrawString(dpy, ROOT, root, 0, H - 2, S, LEN);
}
