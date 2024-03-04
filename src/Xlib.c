#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <string.h>
#include <stdio.h>
#include <Xlib.h>
#include <util.h>
#include <../config.h>

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
static atom_t atom;
static Drawable drawable;
static GC rootgc;
static pair_t dpysize;

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
  return !xerror;
}

void deinit_root() {
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}
/*
void events(volatile sig_atomic_t* sig) {
  void (*EVFN[LASTEvent])(Display*, const Window, const XEvent*);
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

  while (*sig == 0 && XNextEvent(dpy, &xev) == 0) {
    EVFN[xev.type](dpy, rootw, &xev);
    XSync(dpy, false);
  }
}
*/

void init_noop(ev_t* ev) {
  EV[0] = ev;
}

static ev_t* noop() {
  fprintf(stdout, "EV: Unregistered\n");
  return EV[0];
}

void init_mapnotify(ev_t* ev) {
  EV[1] = ev;
}

static ev_t* mapnotify() {
  fprintf(stdout, "EV: Mapnotify\n");
  return EV[1];
}

void init_unmapnotify(ev_t* ev) {
  EV[2] = ev;
}

static ev_t* unmapnotify() {
  fprintf(stdout, "EV: Unmapnotify\n");
  const Window W = xev.xunmap.window;
  //unmap(dpy, ROOTW, W);
  EV[2]->DATA[0] = W;
  return EV[2];
}

void init_clientmessage(ev_t* ev) {
  EV[3] = ev;
}

static ev_t* clientmessage() {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = xev.xclient.window;
  EV[3]->DATA[0] = W;
  return EV[3];
}

void init_configurenotify(ev_t* ev) {
  EV[4] = ev;
}

static ev_t* configurenotify() {
  fprintf(stdout, "EV: Configure Notify\n");
  const Window W = xev.xconfigure.window;
  const int WIDTH = xev.xconfigure.width;
  const int HEIGHT = xev.xconfigure.height;
  // want to reconfigure root window
  EV[4]->DATA[0] = W;
  EV[4]->DATA[1] = WIDTH;
  EV[4]->DATA[2] = HEIGHT;
  return EV[4];
}

void init_maprequest(ev_t* ev) {
  EV[5] = ev;
}

static ev_t* maprequest() {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = xev.xmaprequest.window;
  //map(dpy, ROOTW, W);
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return EV[0];
  
  static const int WMASK = EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask;
  XSelectInput(dpy, W, WMASK);
  XChangeProperty(dpy, rootw, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
    (unsigned char*) &W, 1);

  EV[5]->DATA[0] = W;
  EV[5]->DATA[1] = wa.width;
  EV[5]->DATA[2] = wa.height;
  return EV[5];
}

void init_configurerequest(ev_t*) {

}

static ev_t* configurerequest() {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = &xev.xconfigurerequest;
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(dpy, CONF->window, CONF->value_mask, &wc);
  return EV[0];
}

void init_motionnotify(ev_t*) {

}

static ev_t* motionnotify() {
  return EV[0];
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = xev.xmotion.window;
}

void init_keypress(ev_t* ev) {
  EV[6] = ev;
}

static ev_t* keypress() {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = xev.xkey.state;
  const int CODE = xev.xkey.keycode;
  //key(dpy, ROOTW, STATE, CODE);
  EV[6]->DATA[0] = rootw;
  EV[6]->DATA[1] = STATE;
  EV[6]->DATA[2] = CODE;
  return EV[6];
}

void init_btnpress(ev_t* ev) {
  EV[7] = ev;
}

static ev_t* btnpress() {
  fprintf(stdout, "EV: Btn Press\n");
  const int STATE = xev.xbutton.state;
  const int CODE = xev.xbutton.button;
  const Window W = xev.xbutton.window;
  XUngrabPointer(dpy, CurrentTime);
  EV[7]->DATA[0] = W;
  EV[7]->DATA[1] = STATE;
  EV[7]->DATA[2] = CODE;
  return EV[7];
}

void init_enternotify(ev_t* ev) {
  EV[8] = ev;
}

static ev_t* enternotify() {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = xev.xcrossing.window;
  EV[8]->DATA[0] = W;
  return EV[8];
}

void init_propertynotify(ev_t* ev) {
  EV[9] = ev;
}

static ev_t* propertynotify() {
  fprintf(stdout, "EV: Prop Notify\n");
  const Window W = xev.xproperty.window;
  EV[9]->DATA[0] = W;
  return EV[9];
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

  return EV[0];
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
}

void init_atoms() {
  atom = (atom_t) {
    XInternAtom(dpy, "WM_PROTOCOLS", false),
    XInternAtom(dpy, "WM_NAME", false),
    XInternAtom(dpy, "WM_DELETE_WINDOW", false),
    XInternAtom(dpy, "WM_STATE", false),
    XInternAtom(dpy, "WM_TAKE_FOCUS", false),
    XInternAtom(dpy, "_NET_SUPPORTED", false),
    XInternAtom(dpy, "_NET_WM_STATE", false),
    XInternAtom(dpy, "_NET_WM_NAME", false),
    XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false),
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false),
    XInternAtom(dpy, "_NET_CLIENT_LIST", false),
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false),
    XInternAtom(dpy, "_NET_WM_DESKTOP", false),
    XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false),
    XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false)
  };
}

void appto_clientlist(const Window W) {
  XChangeProperty(dpy, rootw, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
      (const unsigned char*) &W, 1);
}

void delfrom_clientlist() {
  XDeleteProperty(dpy, rootw, atom.CLIENT_LIST);
}

void del_actwindow(const Window W) {
  XDeleteProperty(dpy, W, atom.ACTIVE_WINDOW);
}

void set_bdrcolor(const Window W, const size_t INACTBDR) {
  XSetWindowBorder(dpy, W, INACTBDR);
}

void set_bdrwidth(const Window W, const size_t WIDTH) {
  XSetWindowBorderWidth(dpy, W, WIDTH);
}

void focusin(const Window W) {
  XSetInputFocus(dpy, W, RevertToPointerRoot, CurrentTime);
  XChangeProperty(dpy, rootw, atom.WM_STATE, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom.ACTIVE_WINDOW, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XRaiseWindow(dpy, W);
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

void init_panel() {
  const int SCRN = DefaultScreen(dpy);
  dpysize = (pair_t) { DisplayWidth(dpy, SCRN), DisplayHeight(dpy, SCRN) };
  drawable = XCreatePixmap(dpy, rootw, dpysize.x, dpysize.y, 
    DefaultDepth(dpy, SCRN));
  rootgc = init_gc();
}

void deinit_panel() {
  XFreeGC(dpy, rootgc);
  XFreePixmap(dpy, drawable); 
}

GC init_gc() {
  GC gc = XCreateGC(dpy, rootw, 0, NULL);
  XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
  return gc;
}

void deinit_gc(const GC GC) {
  XFreeGC(dpy, GC);
}

void draw_root(const char* S) {
  XSetForeground(dpy, rootgc, TITLEBG);
  XFillRectangle(dpy, rootw, rootgc, 0, 0, dpysize.x, BARH);
  XSetForeground(dpy, rootgc, TITLEFG);
  XDrawString(dpy, rootw, rootgc, 0, BARH - 2, S, strlen(S));
}
