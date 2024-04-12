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
    PropertyChangeMask |
    ExposureMask
  };

  XSelectInput(dpy, rootw, MASK);
  XUngrabKey(dpy, AnyKey, AnyModifier, rootw);
  return !xerror;
}

void deinit_root() {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
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

Window* init_querytree(unsigned* n) {
  Window root;
  Window par;
  Window* w;
  return XQueryTree(dpy, rootw, &root, &par, &w, n) ? w : NULL;
}

void deinit_querytree(Window* w) {
  XFree(w);
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

bool wa_size(int* w, int* h, const Window W) {
  XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) && !wa.override_redirect &&
    wa.map_state == IsViewable) {
    *w = wa.width;
    *h = wa.height;
    return true;
  }

  return false;
}

void map(const Window W) {
  static const long MASK = { EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask
  };

  XSelectInput(dpy, W, MASK);
  XChangeProperty(dpy, rootw, atom(NET_CLIENT_LIST), XA_WINDOW, 32, 
    PropModeAppend, (unsigned char*) &W, 1);
}

void focusin(const Window W) {
  XSetInputFocus(dpy, W, RevertToPointerRoot, CurrentTime);
  XRaiseWindow(dpy, W);
  XChangeProperty(dpy, rootw, atom(WM_STATE), XA_WINDOW, 32, PropModeReplace, 
    (unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom(NET_ACTIVE_WINDOW), XA_WINDOW, 32, 
    PropModeReplace, (unsigned char*) &W, 1);
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

Window init_window(const unsigned W, const unsigned H) {
  return XCreateSimpleWindow(dpy, rootw, 0, 0, W, H, 0, 0, 0x141414);
}

void deinit_window(const Window W) {
  XDestroyWindow(dpy, W);
}

unsigned dpywidth() {
  return DisplayWidth(dpy, DefaultScreen(dpy));
}

unsigned dpyheight() {
  return DisplayHeight(dpy, DefaultScreen(dpy));
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

void query_screen(const int N, unsigned* x, unsigned* y, unsigned* w, 
  unsigned* h) {
  *x = screeninfo[N].x_org;
  *y = screeninfo[N].y_org;
  *w = screeninfo[N].width;
  *h = screeninfo[N].height;
}
