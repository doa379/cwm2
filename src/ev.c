#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "evcalls.h"
#include "input.h"

extern Display* dpy;

static XEvent xev;
static XMapEvent* const xmap = &xev.xmap;
static XUnmapEvent* const xunmap = &xev.xunmap;
static XClientMessageEvent* const xclient = &xev.xclient;
static XConfigureEvent* const xconfigure = &xev.xconfigure;
static XMapRequestEvent* const xmaprequest = 
  &xev.xmaprequest;
static XDestroyWindowEvent* const xdestroywindow = 
  &xev.xdestroywindow;
static XConfigureRequestEvent const* const xconfrequest =
  &xev.xconfigurerequest;
static XMotionEvent* const xmotion = &xev.xmotion;
static XMappingEvent* const xmapping = &xev.xmapping;
static XKeyEvent* const xkey = &xev.xkey;
static XButtonEvent* const xbutton = &xev.xbutton;
static XCrossingEvent* const xcrossing = &xev.xcrossing;
static XFocusChangeEvent const* xfocus = &xev.xfocus;
static XPropertyEvent* const xproperty = &xev.xproperty;
static XExposeEvent* const xexpose = &xev.xexpose;
static void (*EV[LASTEvent])(void);

static void
ev_noop(void) {
}

static void
ev_map_notify(void) {
  (void) xmap;
}

static void
ev_unmap_notify(void) {
  (void) xunmap;
}

static void
ev_client_message(void) {
  if (xclient->format == 8) {
    evcalls_byte_msg(xclient->window, xclient->message_type,
      xclient->data.b);
  } else if (xclient->format == 16) {
    evcalls_short_msg(xclient->window, xclient->message_type,
      xclient->data.s);
  } else if (xclient->format == 32) {
    evcalls_long_msg(xclient->window, xclient->message_type,
      xclient->data.l);
  }
}

static void
ev_configure_notify(void) {
  if (xconfigure->window == DefaultRootWindow(dpy)) {
    evcalls_configure_root(xconfigure->x, xconfigure->y, 
      xconfigure->width, xconfigure->height);
  }
}

static void
ev_map_request(void) {
  XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, xmaprequest->window, 
      &wa) == 0) {
    return;
  } else if (wa.override_redirect) {
    evcalls_map_override_redirect(
      xmaprequest->window, wa.x, wa.y,
        wa.width, wa.height);
  } else {
    evcalls_map_request(xmaprequest->window, wa.x, wa.y, 
      wa.width, wa.height);
  }
}

static void
ev_destroy_notify(void) {
  evcalls_destroy_notify(xdestroywindow->window);
}

static void
ev_configure_request(void) {
  evcalls_configure_request(xconfrequest->window, 
    xconfrequest->x, xconfrequest->y, 
    xconfrequest->width, xconfrequest->height, 
    xconfrequest->border_width, xconfrequest->value_mask);
}

static void
ev_motion_notify(void) {
  evcalls_motion_notify(xmotion->window, 
    xmotion->x, xmotion->y, xmotion->x_root, 
      xmotion->y_root);
}

static void
ev_mapping_notify(void) {
  XRefreshKeyboardMapping(xmapping);
  if (xmapping->request == MappingKeyboard) {
    input_keys_grab(xmapping->window);
  }
}

static void
ev_key_press(void) {
  evcalls_key_press(xkey->state, xkey->keycode);
}

static void
ev_btn_press(void) {
  evcalls_btn_press(xbutton->window, 
    xbutton->state, xbutton->button, xbutton->x, xbutton->y, 
      xbutton->x_root, xbutton->y_root);
}

static void
ev_enter_notify(void) {
  if (xcrossing->mode == NotifyNormal && 
      xcrossing->detail != NotifyInferior && 
      xcrossing->window != DefaultRootWindow(dpy)) {
    evcalls_enter_notify(xcrossing->window);
  }
}

static void
ev_leave_notify(void) {
  if (xcrossing->mode == NotifyNormal && 
      xcrossing->detail != NotifyInferior && 
      xcrossing->window != DefaultRootWindow(dpy)) {
    evcalls_leave_notify(xcrossing->window);
  }
}

static void
ev_focus_change(void) {
  evcalls_focus_change(xfocus->window);
}

static void
ev_property_notify(void) {
  evcalls_property_notify(xproperty->window, 
    xproperty->atom);
}

static void
ev_expose(void) {
  evcalls_expose(xexpose->window);
}

void
ev_init(void) {
  for (size_t i = 0; i < LASTEvent; i++)
    EV[i] = ev_noop;

  EV[MapNotify] = ev_map_notify;
  EV[UnmapNotify] = ev_unmap_notify;
  EV[ClientMessage] = ev_client_message;
  EV[ConfigureNotify] = ev_configure_notify;
  EV[MapRequest] = ev_map_request;
  EV[DestroyNotify] = ev_destroy_notify;
  EV[ConfigureRequest] = ev_configure_request;
  EV[MotionNotify] = ev_motion_notify;
  EV[MappingNotify] = ev_mapping_notify;
  EV[KeyPress] = ev_key_press;
  EV[ButtonPress] = ev_btn_press;
  EV[EnterNotify] = ev_enter_notify;
  EV[LeaveNotify] = ev_leave_notify;
  EV[FocusIn] = ev_focus_change;
  EV[PropertyNotify] = ev_property_notify;
  EV[Expose] = ev_expose;
}

void
ev_call(void) {
  if (XNextEvent(dpy, &xev) == 0)
    EV[xev.type]();
}

void
ev_motion_drain(void) {
  XSync(dpy, False);
  while (XCheckTypedEvent(dpy, MotionNotify, &xev)) {
  }
}
