#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <Xlib.h>

static Display* dpy;
static Window rootw;
static bool xerror;
static XEvent xev;
static ev_t* (*EVFN[LASTEvent])();
static ev_t* EV[32];
static ev_t* MSGEV[32];
static Atom ATOM[32];

static Drawable drawable;
static const char* FONT = { 
  // These are a dependency for X11 so no checks necessary here
  //"-misc-fixed-medium-r-normal--0-0-100-100-c-0-iso10646-1" };
  "9x15bold" };
static XFontStruct* fn;
static unsigned bh;

static int XError(Display*, XErrorEvent* xev) {
  xerror = xev->error_code == BadAccess;
  return 0;
}

bool init_dpy() {
  return (dpy = XOpenDisplay(NULL));
}

void deinit_dpy() {
  XCloseDisplay(dpy);
}

bool init_root() {
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
  XSetWindowBackground(dpy, rootw, 0x006e99);
  XClearWindow(dpy, rootw);
  return !xerror;
}

void deinit_root() {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XUngrabServer(dpy);
}

void init_event(ev_t* ev) {
  EV[ev->EVENT] = ev;
}

void init_msgevent(ev_t* ev) {
  MSGEV[ev->PROP] = ev;
}

static ev_t* noop() {
  fprintf(stdout, "EV: Unregistered\n");
  return EV[NOOP];
}

static ev_t* mapnotify() {
  fprintf(stdout, "EV: Mapnotify\n");
  return EV[MAPNOTIFY];
}

static ev_t* unmapnotify() {
  fprintf(stdout, "EV: Unmapnotify\n");
  const Window W = { xev.xunmap.window };
  EV[UNMAPNOTIFY]->DATA[0] = W;
  return EV[UNMAPNOTIFY];
}

static ev_t* clientmessage() {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = { xev.xclient.window };
  (void) W;
  const Atom PROP = { xev.xclient.message_type };
  if (PROP == ATOM[WM_PROTOCOLS]) {
    ;
  } else if (PROP == ATOM[WM_NAME]);
  else if (PROP == ATOM[WM_DELETE_WINDOW]);
  else if (PROP == ATOM[WM_STATE]);
  else if (PROP == ATOM[WM_TAKE_FOCUS]);
  else if (PROP == ATOM[NET_SUPPORTED]);
  else if (PROP == ATOM[NET_WM_STATE]);
  else if (PROP == ATOM[NET_WM_NAME]);
  else if (PROP == ATOM[NET_WM_WINDOW_OPACITY]);
  else if (PROP == ATOM[NET_ACTIVE_WINDOW]);
  else if (PROP == ATOM[NET_WM_STATE_FULLSCREEN]);
  else if (PROP == ATOM[NET_WM_WINDOW_TYPE]);
  else if (PROP == ATOM[NET_WM_WINDOW_TYPE_DIALOG]);
  else if (PROP == ATOM[NET_CLIENT_LIST]);
  else if (PROP == ATOM[NET_NUMBER_OF_DESKTOPS]);
  else if (PROP == ATOM[NET_WM_DESKTOP]);
  else if (PROP == ATOM[NET_CURRENT_DESKTOP]) {
    ev_t* ev = { MSGEV[NET_CURRENT_DESKTOP] };
    ev->DATA[0] = xev.xclient.data.l[0];
    return ev;
  } else if (PROP == ATOM[NET_SHOWING_DESKTOP])
    ;

  return EV[NOOP];
}

static ev_t* configurenotify() {
  fprintf(stdout, "EV: Configure Notify\n");
  const Window W = { xev.xconfigure.window };
  const int WIDTH = { xev.xconfigure.width };
  const int HEIGHT = { xev.xconfigure.height };
  // want to reconfigure root window
  if (W == rootw) {
    ev_t* ev = { EV[CONFIGUREROOT] };
    ev->DATA[0] = -1;
    ev->DATA[1] = WIDTH;
    ev->DATA[2] = HEIGHT;
    return ev;
  }

  ev_t* ev = { EV[CONFIGURENOTIFY] };
  ev->DATA[0] = W;
  ev->DATA[1] = WIDTH;
  ev->DATA[2] = HEIGHT;
  return ev;
}

static ev_t* maprequest() {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = { xev.xmaprequest.window };
  XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return EV[NOOP];
  
  static const long MASK = { EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask
  };
  XSelectInput(dpy, W, MASK);
  XChangeProperty(dpy, rootw, ATOM[NET_CLIENT_LIST], XA_WINDOW, 32, 
    PropModeAppend, (unsigned char*) &W, 1);

  ev_t* ev = { EV[MAPREQUEST] };
  ev->DATA[0] = W;
  ev->DATA[1] = wa.width;
  ev->DATA[2] = wa.height;
  return ev;
}

static ev_t* configurerequest() {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = { &xev.xconfigurerequest };
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(dpy, CONF->window, CONF->value_mask, &wc);
  return EV[CONFIGUREREQUEST];
}

static ev_t* motionnotify() {
  return EV[MOTIONNOTIFY];
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = { xev.xmotion.window };
  (void) W;
}

static ev_t* keypress() {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = { xev.xkey.state };
  const int CODE = { xev.xkey.keycode };
  ev_t* ev = { EV[KEYPRESS] };
  ev->DATA[0] = rootw;
  ev->DATA[1] = STATE;
  ev->DATA[2] = CODE;
  return ev;
}

static ev_t* btnpress() {
  fprintf(stdout, "EV: Btn Press\n");
  const int STATE = { xev.xbutton.state };
  const int CODE = { xev.xbutton.button };
  const Window W = { xev.xbutton.window };
  XUngrabPointer(dpy, CurrentTime);
  ev_t* ev = { EV[BTNPRESS] };
  ev->DATA[0] = W;
  ev->DATA[1] = STATE;
  ev->DATA[2] = CODE;
  return ev;
}

static ev_t* enternotify() {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = { xev.xcrossing.window };
  EV[ENTERNOTIFY]->DATA[0] = W;
  return EV[ENTERNOTIFY];
}

static ev_t* propertynotify() {
  fprintf(stdout, "EV: Prop Notify\n");
  const Window W = { xev.xproperty.window };
  EV[PROPERTYNOTIFY]->DATA[0] = W;
  return EV[PROPERTYNOTIFY];
}

void init_events() {
  for (size_t i = { 0 }; i < LASTEvent; i++)
    EVFN[i] = noop;

  EVFN[MapNotify] = mapnotify;
  EVFN[UnmapNotify] = unmapnotify;
  EVFN[ClientMessage] = clientmessage;
  EVFN[ConfigureNotify] = configurenotify;
  EVFN[MapRequest] = maprequest;
  EVFN[ConfigureRequest] = configurerequest;
  EVFN[MotionNotify] = motionnotify;
  EVFN[KeyPress] = keypress;
  EVFN[ButtonPress] = btnpress;
  EVFN[EnterNotify] = enternotify;
  EVFN[PropertyNotify] = propertynotify;
}

ev_t* event() {
  /*
  XSync(dpy, false);
  return XNextEvent(dpy, &xev) == 0 ? EVFN[xev.type]() : EV[NOOP];
  */
  while (XPending(dpy)) {
    if (XNextEvent(dpy, &xev) == 0) {
      const ev_t* EV = { EVFN[xev.type]() };
      EV->evfn(EV->DATA[0], EV->DATA[1], EV->DATA[2]);
    }
  
    XSync(dpy, false);
  }

  return EV[NOOP];
}

void intr_event() {
  close(ConnectionNumber(dpy));
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

int keycode2sym(const int CODE) {
  return XkbKeycodeToKeysym(dpy, CODE, 0, 0);
}

void grab_btn(const Window W, const int MOD, const int KEY) {
  static const long MASK = { ButtonPressMask | ButtonReleaseMask };
  XGrabButton(dpy, KEY, MOD, W, false, MASK, GrabModeSync, GrabModeSync,
    None, None);
}

void ungrab_btn(const Window W, const int MOD, const int KEY) {
  XUngrabButton(dpy, KEY, MOD, W);
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

void init_atoms() {
  ATOM[WM_PROTOCOLS] = XInternAtom(dpy, "WM_PROTOCOLS", false);
  ATOM[WM_NAME] = XInternAtom(dpy, "WM_NAME", false);
  ATOM[WM_DELETE_WINDOW] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
  ATOM[WM_STATE] = XInternAtom(dpy, "WM_STATE", false);
  ATOM[WM_TAKE_FOCUS] = XInternAtom(dpy, "WM_TAKE_FOCUS", false);
  ATOM[NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", false);
  ATOM[NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", false);
  ATOM[NET_WM_NAME] = XInternAtom(dpy, "_NET_WM_NAME", false);
  ATOM[NET_WM_WINDOW_OPACITY] = 
    XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", false);
  ATOM[NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false);
  ATOM[NET_WM_STATE_FULLSCREEN] = 
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false);
  ATOM[NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
  ATOM[NET_WM_WINDOW_TYPE_DIALOG] = 
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false);
  ATOM[NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", false);
  ATOM[NET_NUMBER_OF_DESKTOPS] = 
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false);
  ATOM[NET_WM_DESKTOP] = XInternAtom(dpy, "_NET_WM_DESKTOP", false);
  ATOM[NET_CURRENT_DESKTOP] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false);
  ATOM[NET_SHOWING_DESKTOP] = XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false);
}

void app_clientlist(const Window W) {
  XChangeProperty(dpy, rootw, ATOM[NET_CLIENT_LIST], XA_WINDOW, 32, 
    PropModeAppend, (const unsigned char*) &W, 1);
}

void clear_clientlist() {
  XDeleteProperty(dpy, rootw, ATOM[NET_CLIENT_LIST]);
}

void del_actwindow(const Window W) {
  XDeleteProperty(dpy, W, ATOM[NET_ACTIVE_WINDOW]);
}

void set_nwks(const int NWKS) {
  XChangeProperty(dpy, rootw, ATOM[NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, 
    PropModeReplace, (const unsigned char*) &NWKS, 1);
}

void set_wks(const int N) {
  XChangeProperty(dpy, rootw, ATOM[NET_CURRENT_DESKTOP], XA_CARDINAL, 32,
    PropModeReplace, (const unsigned char*) &N, 1);
}

//////////////////////////////////////////////////////////////////
void app_prop(const Window W, const Atom PROP) {
  XChangeProperty(dpy, rootw, PROP, XA_WINDOW, 32, PropModeAppend, 
    (const unsigned char*) &W, 1);
}

void del_prop(const Window W, const Atom PROP) {
  XDeleteProperty(dpy, W, PROP);
}
  
void del_rootprop(const Atom PROP) {
  XDeleteProperty(dpy, rootw, PROP);
}
//////////////////////////////////////////////////////////////////
void init_ewmh() {
  // Init/Reset EWMH
  XDeleteProperty(dpy, rootw, ATOM[NET_CLIENT_LIST]);
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
  XChangeProperty(dpy, rootw, ATOM[WM_STATE], XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, ATOM[NET_ACTIVE_WINDOW], XA_WINDOW, 32, 
    PropModeReplace, (const unsigned char*) &W, 1);
}
  
bool send_killmsg(const Window W) {
  XEvent xev = { ClientMessage };
  xev.xclient.window = W;
  xev.xclient.message_type = ATOM[WM_PROTOCOLS];
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = ATOM[WM_DELETE_WINDOW];
  xev.xclient.data.l[1] = CurrentTime;
  const Status STATUS = { XSendEvent(dpy, W, false, NoEventMask, &xev) };
  return STATUS == 0 || STATUS == BadValue || STATUS == BadWindow ? false :
    true;
}

bool send_switchwks(const unsigned N) {
  XEvent xev = { ClientMessage };
  xev.xclient.serial = 0;
  xev.xclient.send_event = true;
  xev.xclient.message_type = ATOM[NET_CURRENT_DESKTOP];
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

// Arrangements

void cascade(int* x, int* y, const unsigned X, const unsigned Y) {
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
}

// Drawables

Window init_shadow(const unsigned WIDTH, const unsigned HEIGHT) {
  return XCreateSimpleWindow(dpy, rootw, 0, 0, WIDTH, HEIGHT, 0, 0, 0x141414);
}

void destroy_window(const Window W) {
  XDestroyWindow(dpy, W);
}

void init_drawable() {
  drawable = XCreatePixmap(dpy, rootw, dpywidth(), dpyheight(), dpydepth());
}

void deinit_drawable() {
  XFreePixmap(dpy, drawable); 
}

unsigned dpywidth() {
  return DisplayWidth(dpy, DefaultScreen(dpy));
}

unsigned dpyheight() {
  return DisplayHeight(dpy, DefaultScreen(dpy));
}

unsigned dpydepth() {
  return DefaultDepth(dpy, DefaultScreen(dpy));
}

GC init_gc() {
  GC gc = { XCreateGC(dpy, rootw, 0, NULL) };
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(const GC GC) {
  XFreeGC(dpy, GC);
}

void init_print() {
  fn = XLoadQueryFont(dpy, FONT);
  bh = fn->ascent + fn->descent;
}

void deinit_print() {
  XFreeFont(dpy, fn);
}

void draw_element(const GC GC, const size_t FG, const size_t BG, 
  const unsigned X0, const unsigned Y0, const unsigned X1, const unsigned Y1) {
  XSetForeground(dpy, GC, BG);
  XFillRectangle(dpy, rootw, GC, X0, Y0, X1, Y1);
}

void draw_wks(const char* S, const GC GC, const size_t FG, const size_t BG,
  unsigned* offset) {
  XClearWindow(dpy, rootw);
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(GC, FG, BG, 0, dpyheight() - bh, SW, dpyheight() - bh);
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, rootw, GC, HPAD_PX, dpyheight() - fn->descent, S, SLEN);
  *offset = SW;
}

void draw_root(const char* S, const GC GC, const size_t FG, const size_t BG,
  unsigned* offset) {
  static const unsigned HPAD_PX = { 4 };
  const unsigned SLEN = { strlen(S) };
  const unsigned SW = { XTextWidth(fn, S, SLEN) };
  draw_element(GC, FG, BG, *offset, dpyheight() - bh, dpywidth(), dpyheight());
  XSetForeground(dpy, GC, FG);
  XDrawString(dpy, rootw, GC, *offset + HPAD_PX, dpyheight() - fn->descent, 
    S, SLEN);
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
