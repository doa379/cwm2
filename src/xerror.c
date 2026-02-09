#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <stdio.h>

extern Display* dpy;

static int xerror;

static int xerror_handler(Display* dpy, XErrorEvent* xev) {
  if (xev->error_code == BadAccess ||
      (xev->request_code == X_SetInputFocus && 
        xev->error_code == BadMatch)) {
    xerror = 1;
  }
  
  return 0;
}

int xerror_wmcheck(void) {
  XSetErrorHandler(xerror_handler);
  XSelectInput(dpy, DefaultRootWindow(dpy), 
    SubstructureRedirectMask);
  XSync(dpy, False);
  if (xerror) {
    fprintf(stderr, "Init. error (another wm running?)\n");
    return -1;
  }

  return 0;
}
