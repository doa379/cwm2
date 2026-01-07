#include <unistd.h>

#include "input.h"
#include "mon.h"
#include "wm.h"
#include "wk.h"
#include "panel.h"
#include "status.h"
#include "tray.h"
#include "prop.h"
#include "mascot.h"

extern Display* dpy;

extern wk_t* prevwk;
extern wk_t* currwk;
extern mon_t* currmon;

extern cblk_t mons;
extern wg_t status;

void
evcalls_configure_notify(Window const win, int const w,
int const h) {
  if (win == DefaultRootWindow(dpy)) {
    /* Configure root window */
    mon_mons_clear();
    mon_conf();
    panel_conf();
    tray_conf();
    wm_cli_currmon_move();
    mascot_draw();
  } else {
    cli_t* const c = wm_cli(win);
    if (c && win == c->win) {
      cli_conf(c, w, h);
      panel_icos_arrange(c->wk);
      panel_arrange(c->wk);
    }
  }
}

void
evcalls_map_override_redirect(Window const win) {
  XMapRaised(dpy, win);
  XSetInputFocus(dpy, win, RevertToPointerRoot,
    CurrentTime);
}

void
evcalls_map_request(Window const win, int const x,
int const y, int const w, int const h) {
  cli_t* const c = wm_cli_map(currwk, win, w, h);
  if (c) {
    wg_str_set(&c->ico, prop_ico(win));
    wg_str_set(&c->hdr, prop_name(win));
    wm_cli_switch(c);
    wm_cli_conf(c, w, h);
    wm_cli_arrange(c, x, y);
    panel_icos_arrange(c->wk);
    panel_arrange(c->wk);
  }
}

void
evcalls_destroy_notify(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c) {
    wk_t* const wk = c->wk;
    wm_cli_kill(c);
    panel_icos_arrange(wk);
    panel_arrange(wk);
  }
}

void
evcalls_motion_notify(Window const win, int const x, 
int const y, int const x_root, int const y_root) {
  static int prev_x_root;
  static int prev_y_root;
  if (win == DefaultRootWindow(dpy)) {
    if (abs(x_root - prev_x_root) > 100 ||
      abs(y_root - prev_y_root) > 100) {
      prev_x_root = x_root;
      prev_y_root = y_root;
      currmon = mon_currmon(x_root, y_root);
      char str[16];
      sprintf(str, "Mon %lu", cblk_dist(&mons, currmon));
      status_str_set(str);
      status_focus(wg_ACT);
    }
  }
}

void
evcalls_key_press(unsigned const state, 
unsigned const keycode) {
  input_t const* input = input_key(state, keycode);
  if (input) {
    if (input->cmd) {
      if (fork() == 0) {
        close(ConnectionNumber(dpy));
        char* const args[] = { NULL };
        execvp(input->cmd, args);
      }
    } else if (input->call)
        input->call();
  }
}

void
evcalls_btn_press(Window const win, unsigned const state,
unsigned const button) {
  cli_t* const c = cli(win, currwk);
  if (c) {
    if (c->mode == RES && c->hdr.win == win)
      wm_cli_translate(c);
    else if (c->mode == RES && c->par.win == win)
      wm_cli_resize(c);
    else if (win == c->min.win)
      wm_cli_min(c);
    else if (c->mode == RES && win == c->max.win)
      wm_cli_max(c);
    else if (c->mode == MAX && win == c->res.win)
      wm_cli_res(c);
    else if (win == c->cls.win) {
      wk_t* const wk = c->wk;
      wm_cli_kill(c);
      panel_icos_arrange(wk);
      panel_arrange(wk);
    } else if (win == c->ico.win) {
      wm_cli_switch(c);
      if (c->mode == MIN)
        wm_cli_raise(c);
    } else {
      input_t const* input = input_btn(state, button);
      if (input)
        input->call();
    }
  }
}

void
evcalls_enter_notify(Window const win) {
  cli_t* const c = cli(win, currwk);
  if (c) {
    if (win == c->min.win)
      wg_pixmap_fill(&c->min, wg_SEL);
    else if (win == c->max.win)
      wg_pixmap_fill(&c->max, wg_SEL);
    else if (win == c->res.win)
      wg_pixmap_fill(&c->res, wg_SEL);
    else if (win == c->cls.win)
      wg_pixmap_fill(&c->cls, wg_SEL);
    else if (c != c->wk->currc) {
      wm_cli_switch(c);
      panel_icos_arrange(c->wk);
    }
  }
}

void
evcalls_leave_notify(Window const win) {
  cli_t* const c = cli(win, currwk);
  if (c) {
    unsigned const clr = c == c->wk->currc ? wg_ACT : 
      wg_BG;
    if (win == c->min.win)
      wg_pixmap_fill(&c->min, clr);
    else if (win == c->max.win)
      wg_pixmap_fill(&c->max, clr);
    else if (win == c->res.win)
      wg_pixmap_fill(&c->res, clr);
    else if (win == c->cls.win)
      wg_pixmap_fill(&c->cls, clr);
  }
}

void
evcalls_focus_change(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c && c != currwk->currc) {
    /* notif, don't switch focus */
    if (c->wk == currwk)
      wg_win_bgset(c->ico.win, wg_SEL);
    else 
      wk_wg_focus(c->wk, wg_SEL);

    wg_str_draw(&c->ico, wg_SEL, 0);
  }
}

void
evcalls_property_notify(Window const win) {
  if (win == DefaultRootWindow(dpy)) {
    /* Changes to root name propagate to status */
    status_str_set(prop_root());
  } else {
    cli_t* const c = wm_cli(win);
    if (c) {
      wg_str_set(&c->hdr, prop_name(win));
      wg_str_draw(&c->hdr, c == c->wk->currc ? wg_ACT : 
        wg_BG, c->par.bdrw);
    }
  }
}

void
evcalls_expose(Window const win) {
  if (win == DefaultRootWindow(dpy)) {
    mascot_draw();
  } else if (win == status.win) {
    status_focus(wg_ACT);
  } else {
    cli_t* const c = wm_cli(win);
    if (c)
      cli_wg_focus(c, c == c->wk->currc ? wg_ACT : wg_BG);
  }
}
