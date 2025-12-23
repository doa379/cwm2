#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>

#include "evcalls.h"
#include "input.h"

extern Display* dpy;

static XEvent xev;
static void (*EV[LASTEvent])(void);

static void
ev_noop(void) {

}

static void
ev_map_notify(void) {

}

static void
ev_unmap_notify(void) {
  Window const win = xev.xunmap.window;
  (void) win;
}

static void
ev_client_message(void) {
  Window const win = xev.xclient.window;
  Atom const prop = xev.xclient.message_type;
  if (win == DefaultRootWindow(dpy))
    fprintf(stdout, "Recv prop %ld, msg %ld\n", 
      prop,
      xev.xclient.data.l[0]);
}

static void
ev_configure_notify(void) {
  Window const win = xev.xconfigure.window;
  int const w = xev.xconfigure.width;
  int const h = xev.xconfigure.height;
  evcalls_configure_notify(win);
}

static void
ev_map_request(void) {
  Window const parwin = xev.xmaprequest.parent;
  (void) parwin;
  Window const win = xev.xmaprequest.window;
  fprintf(stdout, "MapRequest Window 0x%lx\n", win);
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, win, &wa) == 0)
    return;
  else if (wa.override_redirect)
    evcalls_map_override_redirect(win);
  else 
    evcalls_map_request(win, wa.x, wa.y, wa.width, 
      wa.height);
}

static void
ev_destroy_notify(void) {
  Window const win = xev.xdestroywindow.window;
  evcalls_destroy_notify(win);
}

static void
ev_configure_request(void) {
  XConfigureRequestEvent const* conf =
    &xev.xconfigurerequest;
  XWindowChanges wc = {
    .x = conf->x,
    .y = conf->y,
    .width = conf->width,
    .height = conf->height,
    .border_width = conf->border_width,
    .sibling = conf->above,
    .stack_mode = conf->detail
  };

  XConfigureWindow(dpy, conf->window, conf->value_mask, 
    &wc);
}

static void
ev_motion_notify(void) {
  Window const win = xev.xmotion.window;
  int const x = xev.xmotion.x;
  int const y = xev.xmotion.y;
  int const x_root = xev.xmotion.x_root;
  int const y_root = xev.xmotion.y_root;
  evcalls_motion_notify(win, x, y, x_root, y_root);
}

static void
ev_mapping_notify(void) {
  XMappingEvent* ev = &xev.xmapping;
  Window const win = xev.xmapping.window;
  XRefreshKeyboardMapping(ev);
  if (ev->request == MappingKeyboard)
    input_keys_grab(win);
}

static void
ev_key_press(void) {
  fprintf(stdout, "EV: Key Press\n");
  Window const win = xev.xkey.window;
  (void) win;
  unsigned const state = xev.xkey.state;
  unsigned const keycode = xev.xkey.keycode;
  evcalls_key_press(state, keycode);
}

static void
ev_btn_press(void) {
  fprintf(stdout, "EV: Btn Press\n");
  Window const win = xev.xbutton.window;
  unsigned const state = xev.xbutton.state;
  unsigned const button = xev.xbutton.button;
  evcalls_btn_press(win, state, button);
}

static void
ev_enter_notify(void) {
  Window const win = xev.xcrossing.window;
  fprintf(stdout, "EV: Enter Notify window 0x%lx\n", win);
  evcalls_enter_notify(win);
}

static void
ev_leave_notify(void) {
  Window const win = xev.xcrossing.window;
  fprintf(stdout, "EV: Leave Notify window 0x%lx\n", win);
  evcalls_leave_notify(win);
}

static void
ev_focus_change(void) {
  XFocusChangeEvent const* xfocus = &xev.xfocus;
  Window const win = xfocus->window;
  int const mode = xfocus->mode;
  (void) mode;
  int const detail = xfocus->detail;
  (void) detail;
  fprintf(stdout, "EV: Focus Change Window 0x%lx\n", win);
  evcalls_focus_change(win);
}

static void
ev_property_notify(void) {
  Window const win = xev.xproperty.window;
  fprintf(stdout, "EV: Prop Notify Window %ld\n", win);
  evcalls_property_notify(win);
}

static void
ev_expose(void) {
  Window const win = xev.xexpose.window;
  fprintf(stdout, "EV: Expose Window 0x%lx\n", win);
  evcalls_expose(win);
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

  XSync(dpy, False);
}
