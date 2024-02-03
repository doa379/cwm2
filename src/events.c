#include <stdio.h>
#include <events.h>

static void noop(XEvent*, const X_t*) {

}

static void mapnotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Mapnotify\n");

}

static void unmapnotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Unmapnotify\n");
  const Window W = xev->xunmap.window;

}

static void clientmessage(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = xev->xclient.window;

}

static void configurenotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Configure Notify\n");
  (void) xev->xconfigure;

}

static void maprequest(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = xev->xmaprequest.window;

}

static void configurerequest(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = &xev->xconfigurerequest;
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(X->dpy, CONF->window, CONF->value_mask, &wc);
}

static void motionnotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = xev->xmotion.window;

}

static void keypress(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Key Press\n");
  const int KEY_STATE = xev->xkey.state,
    KEY_CODE = xev->xkey.keycode;

}

static void btnpress(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Btn Press\n");
  const int BTN_STATE = xev->xbutton.state,
    BTN_CODE = xev->xbutton.button;
  const Window W = xev->xbutton.window;

}

static void enternotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = xev->xcrossing.window;

}

static void propertynotify(XEvent* xev, const X_t* X) {
  fprintf(stdout, "EV: Porp Notify\n");
  const Window W = xev->xproperty.window;

}

void events(const X_t* X, sig_atomic_t* sig_status) {
  static XEvent xev;
  void (*EVFN[LASTEvent])(XEvent*, const X_t*);
  for (int i = 0; i < LASTEvent; i++)
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
    
  while (*sig_status == 0 && XNextEvent(X->dpy, &xev) == 0) {
    EVFN[xev.type](&xev, X);
    XSync(X->dpy, false);
  }
}

