#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <atoms.h>

static Display* dpy;
static Window rootw;
static bool xerror;

static XineramaScreenInfo* screeninfo;

static int XError(Display*, XErrorEvent* xev) {
  xerror = xev->error_code == BadAccess;
  return 0;
}

bool init_root(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
  XSetErrorHandler(XError);
  const long MASK = { 
    SubstructureRedirectMask | 
    SubstructureNotifyMask | 
    ButtonPressMask |
    PointerMotionMask |
    EnterWindowMask |
    LeaveWindowMask |
    StructureNotifyMask |
    PropertyChangeMask
  };

  XSelectInput(dpy, rootw, MASK);
  XUngrabKey(dpy, AnyKey, AnyModifier, rootw);
  return !xerror;
}

void deinit_root() {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XUngrabServer(dpy);
}

int modmask() {
  XModifierKeymap* modmap = { XGetModifierMapping(dpy) };
  unsigned numlockmask = { 0 };
  for (int k = { 0 }; k < 8; k++)
    for (int j = { 0 }; j < modmap->max_keypermod; j++)
      if (modmap->modifiermap[modmap->max_keypermod * k + j] == 
        XKeysymToKeycode(dpy, XK_Num_Lock))
        numlockmask = (1 << k);
  
  XFreeModifiermap(modmap);
  return ~(numlockmask | LockMask);
}

bool xinerama() {
  return XineramaIsActive(dpy);
}

int init_queryscreens() {
  int n;
  screeninfo = XineramaQueryScreens(dpy, &n);
  return n;
}

void deinit_queryscreens() {
  XFree(screeninfo);
}

void query_screen(const int N, unsigned* x, unsigned* y, unsigned* width, 
  unsigned* height) {
  *x = screeninfo[N].x_org;
  *y = screeninfo[N].y_org;
  *width = screeninfo[N].width;
  *height = screeninfo[N].height;
}

void init_windows() {
  Window root;
  Window par;
  Window* w;
  unsigned n;
  XGrabServer(dpy);
  if (XQueryTree(dpy, rootw, &root, &par, &w, &n)) {
    const long MASK = { 
      SubstructureRedirectMask | 
      SubstructureNotifyMask | 
      ButtonPressMask |
      PointerMotionMask |
      EnterWindowMask |
      LeaveWindowMask |
      StructureNotifyMask |
      PropertyChangeMask
    };

    for (unsigned i = { 0 }; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && 
          (wa.map_state == IsViewable || wa.map_state == IconicState)) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = rootw;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, rootw, true, MASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XUngrabServer(dpy);
}

void grab_key(const int MOD, const int KEY) {
  XGrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD, rootw, true, GrabModeAsync, 
    GrabModeAsync);
}

void ungrab_key(const int MOD, const int KEY) {
  XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD, rootw);
}

void grab_btn(const Window W, const int MOD, const int BTN) {
  static const long MASK = { ButtonPressMask | ButtonReleaseMask };
  XGrabButton(dpy, BTN, MOD, W, false, MASK, GrabModeSync, GrabModeSync,
    None, None);
}

void ungrab_btn(const Window W, const int MOD, const int BTN) {
  XUngrabButton(dpy, BTN, MOD, W);
}

void warp_pointer(const unsigned X, const unsigned Y) {
  XWarpPointer(dpy, None, rootw, 0, 0, 0, 0, X, Y);
}

void movewindow(const Window W, const int X, const int Y) {
  XMoveWindow(dpy, W, X, Y);
}

void mapwindow(const Window W) {
  XMapRaised(dpy, W);
}

void unmapwindow(const Window W) {
  XUnmapWindow(dpy, W);
}

void set_bdrcolor(const Window W, const size_t BDR) {
  XSetWindowBorder(dpy, W, BDR);
}

void set_bdrwidth(const Window W, const size_t WIDTH) {
  XSetWindowBorderWidth(dpy, W, WIDTH);
}

void focusin(const Window W) {
  XSetInputFocus(dpy, W, RevertToPointerRoot, CurrentTime);
  XRaiseWindow(dpy, W);
  XChangeProperty(dpy, rootw, atom(WM_STATE), XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom(NET_ACTIVE_WINDOW), XA_WINDOW, 32, 
    PropModeReplace, (const unsigned char*) &W, 1);
}
  
bool send_killmsg(const Window W) {
  XEvent xev = { ClientMessage };
  xev.xclient.window = W;
  xev.xclient.message_type = atom(WM_PROTOCOLS);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = atom(WM_DELETE_WINDOW);
  xev.xclient.data.l[1] = CurrentTime;
  const Status STATUS = { XSendEvent(dpy, W, false, NoEventMask, &xev) };
  return STATUS == 0 || STATUS == BadValue || STATUS == BadWindow ? false :
    true;
}

bool send_switchwks(const unsigned N) {
  XEvent xev = { ClientMessage };
  xev.xclient.serial = 0;
  xev.xclient.send_event = true;
  xev.xclient.message_type = atom(NET_CURRENT_DESKTOP);
  xev.xclient.window = rootw;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = N;
  static const long MASK = { 
    SubstructureRedirectMask | SubstructureNotifyMask
  };
  const Status STATUS = { XSendEvent(dpy, rootw, false, MASK, &xev) };
  return STATUS == 0 || STATUS == BadValue || STATUS == BadWindow ? false :
    true;
}

void spawn(const char* CMD) {
  if (fork() == 0) {
    close(ConnectionNumber(dpy));
    setsid();
    system(CMD);
  }
}

void destroy_window(const Window W) {
  XDestroyWindow(dpy, W);
}

// Arrangements

void cascade(int* x, int* y, const unsigned X, const unsigned Y) {
  /*
  static short grav;
  const char DIRX = { grav >> 0 & 1 ? -1 : 1 };
  const char DIRY = { grav >> 1 & 1 ? -1 : 1 };
  *x += *y != 0 ? DIRX * bh : 0; 
  *y += DIRY * bh;
  if (*x + X > dpywidth()) {
    *x = dpywidth() - X;
    grav |= 1 << 0;
  } if (*y + Y > dpyheight() - bh - 8) {
    *y = dpyheight() - Y - bh - 8;
    grav |= 1 << 1; 
  } if (*x < 0) {
    *x = 0;
    grav ^= 1 << 0;
  } if (*y < bh) {
    *y = bh;
    grav ^= 1 << 1;
  }
  */
}
