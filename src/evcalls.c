#include <unistd.h>

#include "input.h"
#include "mon.h"
#include "wm.h"
#include "wk.h"
#include "panel.h"
#include "status.h"
#include "tray.h"
#include "prop.h"

extern Display* dpy;

extern wk_t* prevwk;
extern wk_t* currwk;
extern mon_t* currmon;

extern cblk_t mons;
extern wg_t status;
extern tray_t tray;

void
evcalls_configure_request(XConfigureRequestEvent const* 
conf) {
  fprintf(stdout, "Client Config Window 0x%lx\n",
    conf->window);
  cli_t* const c = wm_cli(conf->window);
  if (c && c->win == conf->window)
    wm_cli_conf(c, conf->width, conf->height);
  else {
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
}

void
evcalls_configure_notify(Window const win, int const w,
int const h) {
  if (win == DefaultRootWindow(dpy)) {
    fprintf(stdout, "Reconfig root window\n");
    mon_mons_clear();
    mon_conf();
    panel_conf();
    tray_conf();
    wm_cli_currmon_move();
  }
}

void
evcalls_map_override_redirect(Window const win) {
  XMapRaised(dpy, win);
}

void
evcalls_map_request(Window const win, int const x,
int const y, int const w, int const h) {
  cli_t* const c = wm_cli_map(currwk, win);
  if (c) {
    strncpy(c->strico, prop_ico(win), sizeof c->strico - 1);
    wg_str_set(&c->hd0, prop_name(win));
   
    wm_cli_conf(c, w, h);
    wm_cli_switch(c);
    
    /*
    cli_t* const prev = cblk_prev(&c->wk->clis, c);
    int const nextx = prev ? 
      prev->x0 + c->hd0.h + c->par.bdrw : x;
    int const nexty = prev ? 
      prev->y0 + c->hd0.h + c->par.bdrw : y;
    wm_cli_arrange(c, nextx, nexty);
    
    wm_cli_arrange(c, x, y);
    */
    wm_ico_enum(c->wk);
    panel_icos_arrange(c->wk);
    panel_arrange(c->wk);
  }
}

void
evcalls_destroy_notify(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c && c->win == win) {
    wk_t* const wk = c->wk;
    wm_cli_kill(c);
    wm_ico_enum(c->wk);
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
      mon_t* const mon = mon_currmon(x_root, y_root);
      if (mon) {
        char str[16];
        sprintf(str, "Mon %lu", cblk_dist(&mons, mon));
        currmon = mon;
        status_str_set(str);
        status_focus(wg_ACT);
      }
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
unsigned const button, int const x, int const y,
int const x_root, int const y_root) {
  fprintf(stdout, "EV: Btn press Window 0x%lx\n", win);
  cli_t* const c = cli(win, currwk);
  if (c) {
    if (c->mode == RES && c->hd0.win == win)
      wm_cli_translate(c, x_root, y_root);
    else if (c->mode == RES && c->siz.win == win)
      wm_cli_resize(c);
    else if (c->min.win == win)
      wm_cli_min(c);
    else if (c->mode == RES && c->max.win == win)
      wm_cli_max(c);
    else if (c->mode == MAX && c->res.win == win)
      wm_cli_res(c);
    else if (c->cls.win == win) {
      wk_t* const wk = c->wk;
      wm_cli_kill(c);
      panel_icos_arrange(wk);
      panel_arrange(wk);
    } else if (c != currwk->currc && (c->par.win == win || 
        c->ico.win == win)) {
      wm_cli_switch(c);
      if (c->mode == MIN)
        wm_cli_raise(c);
    } else {
      input_t const* input = input_btn(state, button);
      if (input)
        input->call();
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk && wk != currwk) {
      wm_wk_switch(wk);
      panel_icos_arrange(wk);
      panel_arrange(wk);
    }
  }
}

void
evcalls_enter_notify(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c) {
    if (win == c->min.win)
      wg_pixmap_fill(&c->min, wg_SEL);
    else if (win == c->max.win)
      wg_pixmap_fill(&c->max, wg_SEL);
    else if (win == c->res.win)
      wg_pixmap_fill(&c->res, wg_SEL);
    else if (win == c->cls.win)
      wg_pixmap_fill(&c->cls, wg_SEL);
    else if (win == c->siz.win)
      wg_pixmap_fill(&c->siz, wg_SEL);
    else if (win == c->ico.win && c != currwk->currc) {
      wg_win_bgset(c->ico.win, wg_SEL);
      wg_win_bdrset(c->ico.win, wg_SEL);
      wg_str_draw(&c->ico, wg_SEL, 0);
    } else if (c != currwk->currc && c->hd0.win == win) {
      wm_cli_switch(c);
      panel_icos_arrange(c->wk);
    } else if (c->wk != currwk) {
        wm_wk_switch(c->wk);
        wm_cli_switch(c);
        panel_icos_arrange(c->wk);
        panel_arrange(c->wk);
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk && wk != currwk)
      wg_win_bgset(wk->wg.win, wg_SEL);
  }
}

void
evcalls_leave_notify(Window const win) {
  cli_t* const c = cli(win, currwk);
  if (c) {
    unsigned const clr = c == c->wk->currc ? wg_ACT : 
      wg_BG;
    if (c->min.win == win)
      wg_pixmap_fill(&c->min, clr);
    else if (c->max.win == win)
      wg_pixmap_fill(&c->max, clr);
    else if (c->res.win == win)
      wg_pixmap_fill(&c->res, clr);
    else if (c->cls.win == win)
      wg_pixmap_fill(&c->cls, clr);
    else if (c->siz.win == win)
      wg_pixmap_fill(&c->siz, clr);
    else if (c->ico.win == win) {
      int clr = c == currwk->currc ? wg_ACT : wg_BG;
      wg_win_bgset(c->ico.win, clr);
      wg_win_bdrset(c->ico.win, clr);
      wg_str_draw(&c->ico, clr, 0);
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk)
      wg_win_bgset(wk->wg.win, 
        wk == currwk ? wg_ACT : wg_BG);
  }
}

void
evcalls_focus_change(Window const win) {
  fprintf(stdout, "EV: Focus Change Window 0x%lx\n", win);
  cli_t* const c = wm_cli(win);
  if (c && c != currwk->currc) {
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
    if (c && c->win == win) {
      wg_str_set(&c->hd0, prop_name(win));
      wg_str_draw(&c->hd0, c == c->wk->currc ? wg_ACT : 
        wg_BG, c->par.bdrw);
    }
  }
}

void
evcalls_expose(Window const win) {
  if (win == DefaultRootWindow(dpy)) {
  
  } else if (win == status.win) {
    status_focus(wg_ACT);
  } else if (win == tray.wg.win) {
    tray_mascot_conf();
  } else {
    cli_t* const c = wm_cli(win);
    if (c) {
      cli_wg_focus(c, c == currwk->currc ? wg_ACT : wg_BG);
    }
  }
}
