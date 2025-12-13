#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xatom.h>

#include "input.h"
#include "mon.h"
#include "wm.h"
#include "wk.h"
#include "panel.h"
#include "tray.h"
#include "prop.h"

extern Display* dpy;
extern wk_t* currwk;
extern wg_t status;

static XEvent xev;
static void (*EV[LASTEvent])(void);

static void ev_noop(void) {

}

static void ev_map_notify(void) {

}

static void ev_unmap_notify(void) {
  Window const win = xev.xunmap.window;
  (void) win;
}

static void ev_client_message(void) {
  Window const win = xev.xclient.window;
  Atom const prop = xev.xclient.message_type;
  /*
  switch (prop) {
    case XInternAtom(dpy, "WM_PROTOCOLS", false):
    case XInternAtom(dpy, "WM_DELETE_WINDOW", false):
    case XInternAtom(dpy, "WM_STATE", false):
    case XInternAtom(dpy, "WM_TAKE_FOCUS", false):
    case XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false):
    case XInternAtom(dpy, "_NET_SUPPORTED", false):
    case XInternAtom(dpy, "_NET_WM_NAME", false):
    case XInternAtom(dpy, "_NET_WM_STATE", false):
    case XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", false):
    case XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false):
    case XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false):
    case XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false):
    case XInternAtom(dpy, "_NET_CLIENT_LIST", false):
    default:
      break;
  }
  */
  if (win == DefaultRootWindow(dpy))
    fprintf(stdout, "Recv prop %ld, msg %ld\n", 
      prop,
      xev.xclient.data.l[0]);
}

static void ev_configure_notify(void) {
  Window const win = xev.xconfigure.window;
  int const w = xev.xconfigure.width;
  int const h = xev.xconfigure.height;
  if (win == DefaultRootWindow(dpy)) {
    fprintf(stdout, "EV: Configure root window %d, %d\n",
        w, h);
    mon_conf();
    cli_currmon_move();
    panel_conf();
    panel_icos_arrange();
    panel_arrange();
    tray_conf();
  }
}

static void ev_map_request(void) {
  Window const parwin = xev.xmaprequest.parent;
  (void) parwin;
  Window const win = xev.xmaprequest.window;
  fprintf(stdout, "MapRequest Window 0x%lx\n", win);
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, win, &wa) == 0)
    return;
  else if (wa.override_redirect) {
    /* Still need to dress these up */
    XMapRaised(dpy, win);
    XSetInputFocus(dpy, win, RevertToPointerRoot,
      CurrentTime);
  } else {
    cli_t* c = wm_cli_map(win, wa.x, wa.y);
    if (c) {
      wg_str_set(&c->ico, prop_ico(win));
      wg_str_set(&c->hdr, prop_name(win));
      wm_cli_focus(c);
      cli_conf(c, wa.width, wa.height);
      panel_icos_arrange();
      panel_arrange();
    }
  }
}

static void ev_destroy_notify(void) {
  fprintf(stdout, "EV: Destroy Notify\n");
  Window const win = xev.xdestroywindow.window;
  cli_t* const c = cli(win);
  if (c) {
    wm_cli_kill(c);
    panel_icos_arrange();
    panel_arrange();
  }
}

static void ev_configure_request(void) {
  fprintf(stdout, "EV: Config Request\n");
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

static void ev_motion_notify(void) {
  Window const win = xev.xmotion.window;
  int const x = xev.xmotion.x;
  int const y = xev.xmotion.y;
  int const x_root = xev.xmotion.x_root;
  int const y_root = xev.xmotion.y_root;
  /*
  fprintf(stdout, "EV: Motion %lu (%d, %d) root (%d, %d)\n",
      win, x, y, x_root, y_root);
  */
  static int prev_x_root;
  static int prev_y_root;
  if (win == DefaultRootWindow(dpy)) {
    if (abs(x_root - prev_x_root) > 100 ||
      abs(y_root - prev_y_root) > 100) {
      prev_x_root = x_root;
      prev_y_root = y_root;
      unsigned const currmon = mon_currmon(x_root, y_root);
    }
  }
}

static void ev_mapping_notify(void) {
  XMappingEvent* ev = &xev.xmapping;
  Window const win = xev.xmapping.window;
  XRefreshKeyboardMapping(ev);
  if (ev->request == MappingKeyboard)
    input_keys_grab(win);
}

static void ev_key_press(void) {
  fprintf(stdout, "EV: Key Press\n");
  Window const win = xev.xkey.window;
  (void) win;
  unsigned const state = xev.xkey.state;
  unsigned const code = xev.xkey.keycode;
  input_key(state, code);
}

static void ev_btn_press(void) {
  fprintf(stdout, "EV: Btn Press\n");
  Window const win = xev.xbutton.window;
  (void) win;
  unsigned const state = xev.xbutton.state;
  unsigned const code = xev.xbutton.button;
  input_btn(state, code);
}

static void ev_enter_notify(void) {
  Window const win = xev.xcrossing.window;
  fprintf(stdout, "EV: Enter Notify window 0x%lx\n", win);
  cli_t* const c = currwk_cli(win);
  if (c)
    wm_cli_focus(c);
}

static void ev_focus_change(void) {
  XFocusChangeEvent const* xfocus = &xev.xfocus;
  Window const win = xfocus->window;
  int const mode = xfocus->mode;
  (void) mode;
  int const detail = xfocus->detail;
  (void) detail;
  fprintf(stdout, "EV: Focus Change Window 0x%lx\n", win);
  cli_t* const c = cli(win);
  if (c)
    wm_cli_focus(c);
}

static void ev_property_notify(void) {
  Window const win = xev.xproperty.window;
  fprintf(stdout, "EV: Prop Notify Window %ld\n", win);
  if (win == DefaultRootWindow(dpy)) {
    wg_str_set(&status, prop_root());
    panel_status_focus();
    panel_arrange();
    return;
  }

  cli_t* const c = cli(win);
  if (c) {
    wg_str_set(&c->hdr, prop_name(win));
    wg_str_draw(&c->hdr, c == currwk->currc ? ACT : BG,
      c->par.bdrw);
  }
}

static void ev_expose(void) {
  Window const win = xev.xexpose.window;
  /* This excludes exposing override_redirects */
  fprintf(stdout, "EV: Expose Window 0x%lx\n", win);
  cli_t* const c = cli(win);
  if (c)
    cli_wg_focus(c, c == currwk->currc ? ACT : BG);
  else if (win == status.win)
    panel_status_focus();
}

void ev_init(void) {
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
  EV[FocusIn] = ev_focus_change;
  EV[PropertyNotify] = ev_property_notify;
  EV[Expose] = ev_expose;
}

void ev_call(void) {
  if (XNextEvent(dpy, &xev) == 0)
    EV[xev.type]();
}

