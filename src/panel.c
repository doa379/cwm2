#include <panel.h>

static Drawable drawable;
static GC root;
static unsigned H;

void init_panel(const X_t* X, const unsigned BARH) {
  drawable = XCreatePixmap(X->dpy, X->root, X->dpysize.x, X->dpysize.y, 
    DefaultDepth(X->dpy, X->scrn));
  root = init_gc(X);
  H = BARH;
}

void deinit_panel(const X_t* X) {
  XFreeGC(X->dpy, root);
  XFreePixmap(X->dpy, drawable); 
}

GC init_gc(const X_t* X) {
  GC gc = XCreateGC(X->dpy, X->root, 0, NULL);
  XSetLineAttributes(X->dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(const GC GC, const X_t* X) {
  XFreeGC(X->dpy, GC);
}

void draw_root(const X_t* X, const char* S, const size_t LEN, const int FG, 
const int BG) {
  XSetForeground(X->dpy, root, BG);
  XFillRectangle(X->dpy, X->root, root, 0, 0, X->dpysize.x, H);
  XSetForeground(X->dpy, root, FG);
  XDrawString(X->dpy, X->root, root, 0, H - 2, S, LEN);
}
