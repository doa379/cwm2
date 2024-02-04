#include <lib.h>

void init_clients(const X_t* X) {
  unsigned n;
  Window rootw, parw;
  Window* w;
  if (XQueryTree(X->dpy, X->root, &rootw, &parw, &w, &n)) {
    for (int i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(X->dpy, w[i], &wa) && 
          wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = X->root;
        xev.xmaprequest.window = w[i];
        XSendEvent(X->dpy, X->root, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(X->dpy, false);
}

