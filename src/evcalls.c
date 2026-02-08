#include <X11/Xatom.h>
#include <unistd.h>

#include "input.h"
#include "mon.h"
#include "wm.h"
#include "wk.h"
#include "panel.h"
#include "status.h"
#include "tray.h"
#include "prop.h"
#include "root.h"

extern Display* dpy;
extern prop_t prop;

extern wk_t* prevwk;
extern wk_t* currwk;
extern mon_t* currmon;

extern cblk_t mons;
extern wg_t status;
extern tray_t tray;

void
evcalls_configure_request(Window const win, int const x,
  int const y, int const w, int const h, int const bw, 
  long const mask) {
  cli_t* const c = wm_cli(win);
  if (c) {
    if (c->fs) {
        prop_win_config(c->ker.win, x, y, w, h, bw);
    } else {
      int const nx = mask & CWX ? x : c->par.x;
      int const ny = mask & CWY ? y : c->par.y;
      int const nw = mask & CWWidth  ? w : c->ker.w;
      int const nh = mask & CWHeight ? h : c->ker.h;
      wm_cli_ker_conf(c, nw, nh);
      wm_cli_arrange(c, nx, ny);
      prop_win_config(c->ker.win, nx, ny, nw, nh, 0);
    }
  } else {
    XWindowChanges wc = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
      .border_width = bw,
      .sibling = DefaultRootWindow(dpy),
      .stack_mode = Above
    };

    XConfigureWindow(dpy, win, mask, &wc);
  }
}

void
evcalls_configure_root(int const x, int const y, 
  int const w, int const h) {
  /* Reset mons */ 
  mon_mons_clear();
  mon_conf();
  panel_conf();
  tray_conf();
  int x_root;
  int y_root;
  root_ptr_query(&x_root, &y_root);
  mon_t* const mon = mon_currmon(x_root, y_root);
  if (mon) {
    currmon = mon;
  }

  wm_cli_currmon_move();
}

void
evcalls_map_override_redirect(Window const win, int const x,
int const y, int const w, int const h) {
  if (wm_ord(win)) {
    /* Discount duplicates */
    return;
  }
  Window* const c = wm_ord_map(win);
  if (c) {
    wm_ord_conf(win, w, h);
    wm_ord_arrange(win, x, y);
  }
}

void
evcalls_map_request(Window const win, int const x,
int const y, int const w, int const h) {
  if (wm_cli(win)) {
    /* Discount duplicates */
    return;
  }

  cli_t* const c = wm_cli_map(currwk, win);
  if (c) {
    wm_cli_ker_conf(c, w, h);
    wm_cli_arrange(c, x, y);
    /* Floats are not yet init'd */
    prop_win_config(c->ker.win, x, y, w, h, 0);
    XMapWindow(dpy, win);
    prop_state_set(win, NormalState);
    wg_str_set(&c->hd0, prop_name(win));
    strncpy(c->strico, prop_ico(win), sizeof c->strico - 1);
    wm_ico_enum(c->wk);
    panel_icos_arrange(c->wk);
    panel_arrange(c->wk);
    wm_cli_switch(c);
  }
}

void
evcalls_destroy_notify(Window const win) {
  cli_t* c = wm_cli(win);
  if (c && c->ker.win == win) {
    wk_t* const wk = c->wk;
    wm_cli_del(c);
    prop_state_set(win, WithdrawnState);
    wm_ico_enum(wk);
    panel_icos_arrange(wk);
    panel_arrange(wk);
    return;
  }

  wg_t* const wg = tray_cli(win);
  if (wg) {
    tray_cli_unmap(wg);
    return;
  }
  
  Window* const ord_win = wm_ord(win);
  if (ord_win) {
    wm_ord_unmap(ord_win);
    return;
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
    } else if (input->call) {
      input->call();
    }
  }
}

void
evcalls_btn_press(Window const win, unsigned const state,
unsigned const button, int const x, int const y,
int const x_root, int const y_root) {
  cli_t* const c = wm_cli(win);
  if (c) {
    if (c->mode == cli_RES && c->hd0.win == win) {
      wm_cli_translate(c, x_root, y_root);
      /* Just in case pinned to tray */
      panel_icos_arrange(c->wk);
      panel_arrange(c->wk);
    } else if (c->siz.win == win) {
      wm_cli_resize(c);
    } else if (c->min.win == win) {
      wm_cli_min(c);
    } else if (c->max.win == win) {
      wm_cli_max(c);
    } else if (c->res.win == win) {
      wm_cli_res(c);
    } else if (c->cls.win == win) {
      prop_win_del(c->ker.win);
    } else if (c->wk != currwk && c->ico.win == win) {
      wm_wk_switch(c->wk);
      panel_icos_arrange(c->wk);
      panel_arrange(c->wk);
      if (c->mode == cli_MIN) {
        wm_cli_raise(c);
      }
      
      wm_cli_switch(c);
    } else if (c->ico.win == win && c != c->wk->currc) {
      if (c->mode == cli_MIN) {
        wm_cli_raise(c);
      }
      
      wm_cli_switch(c);
      XMapRaised(dpy, c->par.win);
    } else {
      input_t const* input = input_btn(state, button);
      if (input) {
        input->call();
      }
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk && wk != currwk) {
      wm_wk_switch(wk);
      panel_icos_arrange(wk);
      panel_arrange(wk);
      return;
    }

    wg_t* const cli = tray_cli(win);
    if (cli) {
      wm_tray_cli_unmap(cli);
      return;
    }
  }
}

void
evcalls_enter_notify(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c) {
    if (win == c->min.win) {
      wg_pixmap_fill(&c->min, wg_SEL);
    } else if (win == c->max.win) {
      wg_pixmap_fill(&c->max, wg_SEL);
    } else if (win == c->res.win) {
      wg_pixmap_fill(&c->res, wg_SEL);
    } else if (win == c->cls.win) {
      wg_pixmap_fill(&c->cls, wg_SEL);
    } else if (win == c->siz.win) {
      wg_pixmap_fill(&c->siz, wg_SEL);
    } else if (win == c->ico.win && 
      (c != currwk->currc || c->mode == cli_MIN)) {
      cli_ico_clr(c, wg_SEL);
    } else if (c != c->wk->currc) {
      wm_cli_switch(c);
      panel_icos_arrange(c->wk);
    } else if (c->hd0.win == win) {
      XRaiseWindow(dpy, c->par.win);
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk && wk != currwk) {
      wg_win_bgclr(wk->wg.win, wg_SEL);
      return;
    }
    
    wg_t* const c = tray_cli(win);
    if (c) {
      XRaiseWindow(dpy, c->win);
      wg_win_bdrclr(c->win, wg_ACT);
      return;
    }
  }
}

void
evcalls_leave_notify(Window const win) {
  cli_t* const c = wm_cli(win);
  if (c) {
    unsigned const clr = 
      c == currwk->currc ? wg_ACT : wg_BG;
    if (c->min.win == win) {
      wg_pixmap_fill(&c->min, clr);
    } else if (c->max.win == win) {
      wg_pixmap_fill(&c->max, clr);
    } else if (c->res.win == win) {
      wg_pixmap_fill(&c->res, clr);
    } else if (c->cls.win == win) {
      wg_pixmap_fill(&c->cls, clr);
    } else if (c->siz.win == win) {
      wg_pixmap_fill(&c->siz, clr);
    } else if (c->ico.win == win) {
      cli_ico_clr(c, clr);
    }
  } else {
    wk_t* const wk = wm_wk(win);
    if (wk) {
      wg_win_bgclr(wk->wg.win, 
        wk == currwk ? wg_ACT : wg_BG);
      return;
    }
    
    wg_t* const c = tray_cli(win);
    if (c) {
      wg_win_bdrclr(c->win, wg_BG);
      return;
    }
  }
}

void
evcalls_focus_change(Window const win) {
}

void
evcalls_property_notify(Window const win, Atom const atom) {
  if (win == DefaultRootWindow(dpy)) {
    if (atom == XA_WM_NAME || atom == prop.net_name) {
      status_str_set(prop_root());
    }
  } else {
    cli_t* const c = wm_cli(win);
    if (c) {
      if (atom == XA_WM_NAME || atom == prop.net_name) {
        wg_str_set(&c->hd0, prop_name(win));
        int const clr = c == c->wk->currc ? wg_ACT : wg_BG;
        cli_clr(c, clr);
        strncpy(c->strico, prop_ico(win), 
          sizeof c->strico - 1);
        wm_cli_ico_enum(c);
        cli_ico_clr(c, clr);
      }
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
      int const clr = c == currwk->currc ? wg_ACT : wg_BG;
      cli_clr(c, clr);
      cli_ico_clr(c, clr);
    }
  }
}

void
evcalls_byte_msg(Window const win, Atom const atom,
char const b[]) {
}

void
evcalls_short_msg(Window const win, Atom const atom,
short const s[]) {
}

void
evcalls_long_msg(Window const win, Atom const atom,
long const l[]) {
  cli_t* const c = wm_cli(win);
  if (c) {
    if (atom == prop.net_state) {
      if (l[1] == prop.net_fs || l[2] == prop.net_fs) {
        if (l[0] == _NET_WM_STATE_ADD ||
            (l[0] == _NET_WM_STATE_TOGGLE && c->fs == 0)) {
          wm_cli_fs(c);
          XChangeProperty(dpy, c->ker.win, prop.net_state, 
            XA_ATOM, 32, PropModeReplace, 
              (unsigned char*) &prop.net_fs, 1);
          c->fs = 1;
        } else if (l[0] == _NET_WM_STATE_REMOVE ||
            (l[0] == _NET_WM_STATE_TOGGLE && c->fs == 1)) {
          XChangeProperty(dpy, c->ker.win, prop.net_state, 
            XA_ATOM, 32, PropModeReplace, NULL, 0);
          c->fs = 0;
          if (c->mode == cli_MAX) {
            wm_cli_max(c);
          } else {
            wm_cli_res(c);
          }
        }
      }
    } else if (atom == prop.net_actwin) {
      /* Show urgency */
      cli_clr(c, wg_SEL);
    }
  }
}
