#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ev.h>
#include <atoms.h>
#include <Xlib.h>

static Display* dpy;
static Window rootw;
static XEvent xev;
static ev_t* (*EVFN[LASTEvent])();
static ev_t* EV[32];
static ev_t* MSGEV[32];

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
  if (PROP == atom(WM_PROTOCOLS)) {
    ;
  } else if (PROP == atom(WM_NAME));
  else if (PROP == atom(WM_DELETE_WINDOW));
  else if (PROP == atom(WM_STATE));
  else if (PROP == atom(WM_TAKE_FOCUS));
  else if (PROP == atom(NET_SUPPORTED));
  else if (PROP == atom(NET_WM_STATE));
  else if (PROP == atom(NET_WM_NAME));
  else if (PROP == atom(NET_WM_WINDOW_OPACITY));
  else if (PROP == atom(NET_ACTIVE_WINDOW));
  else if (PROP == atom(NET_WM_STATE_FULLSCREEN));
  else if (PROP == atom(NET_WM_WINDOW_TYPE));
  else if (PROP == atom(NET_WM_WINDOW_TYPE_DIALOG));
  else if (PROP == atom(NET_CLIENT_LIST));
  else if (PROP == atom(NET_NUMBER_OF_DESKTOPS));
  else if (PROP == atom(NET_WM_DESKTOP));
  else if (PROP == atom(NET_CURRENT_DESKTOP)) {
    ev_t* ev = { MSGEV[NET_CURRENT_DESKTOP] };
    ev->DATA[0] = xev.xclient.data.l[0];
    return ev;
  } else if (PROP == atom(NET_SHOWING_DESKTOP))
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
    ev->DATA[0] = WIDTH;
    ev->DATA[1] = HEIGHT;
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
  XChangeProperty(dpy, rootw, atom(NET_CLIENT_LIST), XA_WINDOW, 32, 
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
  const Window W = { xev.xmotion.window };
  ev_t* ev = { EV[MOTIONNOTIFY] };
  ev->DATA[0] = xev.xmotion.x_root;
  ev->DATA[1] = xev.xmotion.y_root;
  return W == rootw ? ev : EV[NOOP];
}

static ev_t* keypress() {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = { xev.xkey.state };
  const int CODE = { xev.xkey.keycode };
  ev_t* ev = { EV[KEYPRESS] };
  /*
  ev->DATA[0] = STATE;
  ev->DATA[1] = CODE;
  */
  ev->DATA[0] = STATE & modmask();
  ev->DATA[1] = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  return ev;
}

static ev_t* btnpress() {
  fprintf(stdout, "EV: Btn Press\n");
  const Window W = { xev.xbutton.window };
  const int STATE = { xev.xbutton.state };
  const int CODE = { xev.xbutton.button };
  XUngrabPointer(dpy, CurrentTime);
  ev_t* ev = { EV[BTNPRESS] };
  ev->DATA[0] = W;
  ev->DATA[1] = STATE & modmask();
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

void init_events(Display* dpy_) {
  dpy = dpy_;
  rootw = XRootWindow(dpy, DefaultScreen(dpy));
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
