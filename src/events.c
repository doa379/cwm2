#include <stdio.h>
#include <events.h>
#include <wm.h>

static XEvent xev;
static Display* dpy;
static Window root;

static void noop() {
  fprintf(stdout, "EV: Unregistered\n");
}

static void mapnotify() {
  fprintf(stdout, "EV: Mapnotify\n");

}

static void unmapnotify() {
  fprintf(stdout, "EV: Unmapnotify\n");
  const Window W = xev.xunmap.window;

}

static void clientmessage() {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = xev.xclient.window;

}

static void configurenotify() {
  fprintf(stdout, "EV: Configure Notify\n");
  (void) xev.xconfigure;

}

static void maprequest() {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = xev.xmaprequest.window;
  map(dpy, root, W);
}

static void configurerequest() {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = &xev.xconfigurerequest;
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(dpy, CONF->window, CONF->value_mask, &wc);
}

static void motionnotify() {
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = xev.xmotion.window;

}

static void keypress() {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = xev.xkey.state;
  const int CODE = xev.xkey.keycode;
  key(dpy, root, STATE, CODE);
}

static void btnpress() {
  fprintf(stdout, "EV: Btn Press\n");
  const int STATE = xev.xbutton.state;
  const int CODE = xev.xbutton.button;
  const Window W = xev.xbutton.window;

}

static void enternotify() {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = xev.xcrossing.window;

}

static void propertynotify() {
  fprintf(stdout, "EV: Prop Notify\n");
  const Window W = xev.xproperty.window;

}

void events(Display* dpy_, const Window ROOT, volatile sig_atomic_t* sig_status) {
  dpy = dpy_;
  root = ROOT;
  void (*EVFN[LASTEvent])();
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

  while (*sig_status == 0 && XNextEvent(dpy, &xev) == 0) {
    EVFN[xev.type]();
    XSync(dpy, false);
  }
}
