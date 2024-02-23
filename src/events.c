#include <stdio.h>
#include <events.h>
#include <wm.h>

static XEvent xev;

static void noop(Display*, const Window) {
  fprintf(stdout, "EV: Unregistered\n");
}

static void mapnotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Mapnotify\n");

}

static void unmapnotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Unmapnotify\n");
  const Window W = xev.xunmap.window;

}

static void clientmessage(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Client Message\n");
  const Window W = xev.xclient.window;

}

static void configurenotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Configure Notify\n");
  const Window W = xev.xconfigure.window;
  if (W == ROOTW) {
    const int WIDTH = xev.xconfigure.width;
    const int HEIGHT = xev.xconfigure.height;
    configure_root(dpy, ROOTW, WIDTH, HEIGHT);
  }
}

static void maprequest(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Map Request\n");
  const Window W = xev.xmaprequest.window;
  map(dpy, ROOTW, W);
}

static void configurerequest(Display* dpy, const Window) {
  fprintf(stdout, "EV: Config Request\n");
  const XConfigureRequestEvent* CONF = &xev.xconfigurerequest;
  XWindowChanges wc = {
    CONF->x, CONF->y, CONF->width, CONF->height,
    CONF->border_width, CONF->above, CONF->detail };
  XConfigureWindow(dpy, CONF->window, CONF->value_mask, &wc);
}

static void motionnotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Motion Notify\n");
  const Window W = xev.xmotion.window;

}

static void keypress(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Key Press\n");
  const int STATE = xev.xkey.state;
  const int CODE = xev.xkey.keycode;
  key(dpy, ROOTW, STATE, CODE);
}

static void btnpress(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Btn Press\n");
  const int STATE = xev.xbutton.state;
  const int CODE = xev.xbutton.button;
  const Window W = xev.xbutton.window;

}

static void enternotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Enter Notify\n");
  const Window W = xev.xcrossing.window;

}

static void propertynotify(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "EV: Prop Notify\n");
  const Window W = xev.xproperty.window;

}

void events(Display* dpy, const Window W, volatile sig_atomic_t* sig_status) {
  void (*EVFN[LASTEvent])(Display*, const Window);
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
    EVFN[xev.type](dpy, W);
    XSync(dpy, false);
  }
}
