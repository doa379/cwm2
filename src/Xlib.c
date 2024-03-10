#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <Xlib.h>
#include <util.h>

static const int ROOTMASK = { 
  SubstructureRedirectMask | 
  SubstructureNotifyMask | 
  ButtonPressMask |
  PointerMotionMask |
  EnterWindowMask |
  LeaveWindowMask |
  StructureNotifyMask |
  PropertyChangeMask
};

typedef struct {
  Atom PROTO;
  Atom NAME;
  Atom DELETE_WINDOW;
  Atom STATE;
  Atom TAKE_FOCUS;
  Atom SUPPORTED;
  Atom WM_STATE;
  Atom WM_NAME;
  Atom WM_WINDOW_OPACITY;
  Atom ACTIVE_WINDOW;
  Atom WM_STATE_FULLSCREEN;
  Atom WM_WINDOW_TYPE;
  Atom WM_WINDOW_TYPE_DIALOG;
  Atom CLIENT_LIST;
  Atom NUMBER_OF_DESKTOPS;
  Atom WM_DESKTOP;
  Atom CURRENT_DESKTOP;
  Atom SHOWING_DESKTOP;
} atom_t;

static Display* dpy;
static Window rootw;
static bool xerror;
static XEvent xev;
static ev_t* (*EVFN[LASTEvent])();
static ev_t* EV[LASTEvent];
// TODO
static atom_t atom;
static Drawable drawable;

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
  XSelectInput(dpy, rootw, ROOTMASK);
  XUngrabKey(dpy, AnyKey, AnyModifier, rootw);
  XSetWindowBackground(dpy, rootw, 0x002531);
  XClearWindow(dpy, rootw);
  return !xerror;
}

void deinit_root() {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void init_event(ev_t* ev) {
  EV[ev->name] = ev;
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
  const Window W = xev.xunmap.window;
  EV[UNMAPNOTIFY]->DATA[0] = W;
  return EV[UNMAPNOTIFY];
}

static ev_t* clientmessage() {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = xev.xclient.window;
  EV[CLIENTMESSAGE]->DATA[0] = W;
  return EV[CLIENTMESSAGE];
}

static ev_t* configurenotify() {
  fprintf(stdout, "EV: Configure Notify\n");
  const Window W = xev.xconfigure.window;
  const int WIDTH = xev.xconfigure.width;
  const int HEIGHT = xev.xconfigure.height;
  // want to reconfigure root window
  ev_t* ev = EV[CONFIGURENOTIFY];
  ev->DATA[0] = W;
  ev->DATA[1] = WIDTH;
  ev->DATA[2] = HEIGHT;
  return ev;
}

static ev_t* maprequest() {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = xev.xmaprequest.window;
  XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return EV[NOOP];
  
  static const int WMASK = EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask;
  XSelectInput(dpy, W, WMASK);
  XChangeProperty(dpy, rootw, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
    (unsigned char*) &W, 1);

  ev_t* ev = EV[MAPREQUEST];
  ev->DATA[0] = W;
  ev->DATA[1] = wa.width;
  ev->DATA[2] = wa.height;
  return ev;
}

static ev_t* configurerequest() {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = &xev.xconfigurerequest;
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(dpy, CONF->window, CONF->value_mask, &wc);
  return EV[CONFIGUREREQUEST];
}

static ev_t* motionnotify() {
  return EV[MOTIONNOTIFY];
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = xev.xmotion.window;
  (void) W;
}

static ev_t* keypress() {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = xev.xkey.state;
  const int CODE = xev.xkey.keycode;
  ev_t* ev = EV[KEYPRESS];
  ev->DATA[0] = rootw;
  ev->DATA[1] = STATE;
  ev->DATA[2] = CODE;
  return ev;
}

static ev_t* btnpress() {
  fprintf(stdout, "EV: Btn Press\n");
  const int STATE = xev.xbutton.state;
  const int CODE = xev.xbutton.button;
  const Window W = xev.xbutton.window;
  XUngrabPointer(dpy, CurrentTime);
  ev_t* ev = EV[BTNPRESS];
  ev->DATA[0] = W;
  ev->DATA[1] = STATE;
  ev->DATA[2] = CODE;
  return ev;
}

static ev_t* enternotify() {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = xev.xcrossing.window;
  EV[ENTERNOTIFY]->DATA[0] = W;
  return EV[ENTERNOTIFY];
}

static ev_t* propertynotify() {
  fprintf(stdout, "EV: Prop Notify\n");
  const Window W = xev.xproperty.window;
  EV[PROPERTYNOTIFY]->DATA[0] = W;
  return EV[PROPERTYNOTIFY];
}

void init_events() {
  for (size_t i = 0; i < LASTEvent; i++)
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
  if (XNextEvent(dpy, &xev) == 0) {
    XSync(dpy, false);
    return EVFN[xev.type]();
  }

  return EV[NOOP];
}

int modmask() {
  XModifierKeymap* modmap = XGetModifierMapping(dpy);
  unsigned numlockmask = { 0 };
  for (int k = 0; k < 8; k++)
    for (int j = 0; j < modmap->max_keypermod; j++)
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
  if (XQueryTree(dpy, rootw, &root, &par, &w, &n)) {
    for (unsigned i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = rootw;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, rootw, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
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
  static const int BUTTONMASK = ButtonPressMask | ButtonReleaseMask;
  XGrabButton(dpy, KEY, MOD, W, false, BUTTONMASK, GrabModeSync, GrabModeSync,
    None, None);
}

void ungrab_btn(const Window W, const int MOD, const int KEY) {
  XUngrabButton(dpy, KEY, MOD, W);
}

void movewindow(const Window W, const int X, const int Y) {
  XMoveWindow(dpy, W, X, Y);
}

void mapwindow(const Window W) {
  XMapWindow(dpy, W);
  XRaiseWindow(dpy, W);
}

void unmapwindow(const Window W) {
  XUnmapWindow(dpy, W);
}

void init_atoms() {
  atom = (atom_t) {
    .PROTO = XInternAtom(dpy, "WM_PROTOCOLS", false),
    .NAME = XInternAtom(dpy, "WM_NAME", false),
    .DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", false),
    .STATE = XInternAtom(dpy, "WM_STATE", false),
    .TAKE_FOCUS = XInternAtom(dpy, "WM_TAKE_FOCUS", false),
    .SUPPORTED = XInternAtom(dpy, "_NET_SUPPORTED", false),
    .WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", false),
    .WM_NAME = XInternAtom(dpy, "_NET_WM_NAME", false),
    .WM_WINDOW_OPACITY = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", false),
    .ACTIVE_WINDOW = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false),
    .WM_STATE_FULLSCREEN = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
    .WM_WINDOW_TYPE = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false),
    .WM_WINDOW_TYPE_DIALOG = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false),
    .CLIENT_LIST = XInternAtom(dpy, "_NET_CLIENT_LIST", false),
    .NUMBER_OF_DESKTOPS = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false),
    .WM_DESKTOP = XInternAtom(dpy, "_NET_WM_DESKTOP", false),
    .CURRENT_DESKTOP = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false),
    .SHOWING_DESKTOP = XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false)
  };
}

void app_clientlist(const Window W) {
  XChangeProperty(dpy, rootw, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
    (const unsigned char*) &W, 1);
}

void clear_clientlist() {
  XDeleteProperty(dpy, rootw, atom.CLIENT_LIST);
}

void del_actwindow(const Window W) {
  XDeleteProperty(dpy, W, atom.ACTIVE_WINDOW);
}

void set_nwks(const int NWKS) {
  XChangeProperty(dpy, rootw, atom.NUMBER_OF_DESKTOPS, XA_CARDINAL, 32, 
    PropModeReplace, (const unsigned char*) &NWKS, 1);
}

void set_wks(const int N) {
  XChangeProperty(dpy, rootw, atom.CURRENT_DESKTOP, XA_CARDINAL, 32,
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
  XDeleteProperty(dpy, rootw, atom.CLIENT_LIST);
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
  XChangeProperty(dpy, rootw, atom.WM_STATE, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom.ACTIVE_WINDOW, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
}
  
void send_killmsg(const Window W) {
  XEvent xev = { ClientMessage };
  xev.xclient.window = W;
  xev.xclient.message_type = atom.PROTO;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = atom.DELETE_WINDOW;
  xev.xclient.data.l[1] = CurrentTime;
  XSendEvent(dpy, W, false, NoEventMask, &xev);
}

void send_switchwks(const unsigned N) {
  fprintf(stdout, "Switch WKS %d\n", N);
}

void spawn(const char* CMD) {
  if (fork() == 0) {
    close(ConnectionNumber(dpy));
    setsid();
    system(CMD);
  }
}

Window init_shadow(const unsigned WIDTH, const unsigned HEIGHT) {
  return XCreateSimpleWindow(dpy, rootw, 0, 0, WIDTH, HEIGHT, 0, 0, 0x141414);
}

void destroy_window(const Window W) {
  XDestroyWindow(dpy, W);
}

void init_drawable() {
  drawable = XCreatePixmap(dpy, rootw, dpywidth(), dpyheight(), 
    DefaultDepth(dpy, DefaultScreen(dpy)));
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

GC init_gc() {
  GC gc = XCreateGC(dpy, rootw, 0, NULL);
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(const GC GC) {
  XFreeGC(dpy, GC);
}

void draw_element(const GC GC, const size_t FG, const size_t BG, 
const unsigned X0, const unsigned Y0, const unsigned X1, const unsigned Y1) {
  XSetForeground(dpy, GC, BG);
  XFillRectangle(dpy, rootw, GC, X0, Y0, X1, Y1);
  XSetForeground(dpy, GC, FG);
}

void print_element(const GC GC, const char* S, const unsigned X, 
  const unsigned HPAD, const unsigned Y, const unsigned VPAD) {
  XDrawString(dpy, rootw, GC, X + HPAD, Y - VPAD, S, strlen(S));
}
