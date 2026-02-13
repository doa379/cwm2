#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "wm.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "input.h"
#include "font.h"
#include "root.h"
#include "tray.h"

extern Display* dpy;
extern font_t font;
extern cblk_t mons;
extern tray_t tray;

static size_t const NRES_ORD = 10;
static cblk_t ord;

cblk_t wks;
wk_t* prevwk;
wk_t* currwk;
mon_t* currmon;

int
wm_init(unsigned const n) {
  wks = cblk_init(sizeof(wk_t), n > 0 ? n : 1);
  if (wks.blk == NULL) {
    fprintf(stderr, "Failed to alloc wks\n");
    return -1;
  }
  /* override redirects */
  ord = cblk_init(sizeof(Window), NRES_ORD);
  if (ord.blk == NULL) {
    fprintf(stderr, "Failed to alloc ord\n");
    return -1;
  }

  for (unsigned i = 0; i < n; i++)
    if (wm_wk_map() == NULL) {
      fprintf(stderr, "Failed to alloc wk\n");
      return -1;
    }

  wm_wk_focus(wks.front);
  prevwk = currwk = wks.front;
  mon_conf();
  currmon = mons.front;
  return 0;
}

void
wm_deinit(void) {
  cblk_deinit(&ord);
  wk_t* wk;
  while ((wk = cblk_back(&wks))) {
    /* Destroy from the end */
    cli_t* c;
    while ((c = cblk_back(&wk->clis))) {
      wm_cli_unmap(c);
    }

    wm_wk_deinit(wk);
  }
  
  cblk_deinit(&wks);
}

wk_t*
wm_wk(Window const win) {
  wk_t* wk = wks.front;
  do {
    if (wk->wg.win == win) {
      return wk;
    }

    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);

  return NULL;
}

wk_t*
wm_wk_map(void) {
  wk_t const wk = wk_init();
  size_t const currwk_ = cblk_dist(&wks, currwk);
  wk_t* const nextwk = cblk_map(&wks, &wk);
  if (nextwk) {
    prevwk = cblk_itr(&wks, currwk_);
    currwk = nextwk;
    return currwk;
  }

  return NULL;
}

void
wm_wk_deinit(wk_t* const wk) {
  wk_deinit(wk);
  cblk_unmap(&wks, wk);
}

int
wm_wk_unmap(wk_t* const wk) {
  if (wks.size == 1 ||
      wk->clis.size == 0) {
    return -1;
  }

  wk_t* const nextwk = cblk_next(&wks, wk);
  while (wk->clis.size) {
    cli_t* const c = wk->clis.front;
    wm_cli_move(c, nextwk);
  }

  if (prevwk == currwk) {
    prevwk = nextwk;
  }

  wm_wk_focus(nextwk);
  currwk = nextwk;
  wm_wk_deinit(wk);
  return 0;
}

void
wm_wk_unfocus(wk_t* const wk) {
  if (wk->clis.size) {
    cli_t* c = wk->clis.front;
    do {
      wm_cli_unfocus(c);
      if (c->stk == 0) {
        XUnmapWindow(dpy, c->par.win);
      }

      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front); 
  }

  wk_bdrclr(wk, wg_ACT);
}

void
wm_wk_focus(wk_t* const wk) {
  if (wk->clis.size) {
    cli_t* c = wk->clis.front; 
    do {
      if (c->mode != cli_MIN) {
        XMapWindow(dpy, c->par.win);
      }

      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front);
  }
  
  wk_bdrclr(wk, wg_SEL);
  if (wk->currc) {
    wm_cli_focus(wk->currc);
  }
}

void
wm_wk_switch(wk_t* const wk) {
  wm_wk_unfocus(currwk);
  prevwk = currwk;
  wm_wk_focus(wk);
  if (wk->clis.size == 0) {
    for (unsigned i = 0; i < 20; i++) {
      usleep(5000);
      wk_clr(wk, wg_SEL);
      XFlush(dpy);
      usleep(5000);
      wk_clr(wk, wg_BG);
      XFlush(dpy);
    }
  }

  currwk = wk;
}

cli_t*
wm_cli_map(wk_t* const wk, Window const win) {
  cli_t const c = cli_init(win, wk);
  return cblk_map(&wk->clis, &c);
}

void
wm_cli_unmap(cli_t* const c) {
  wm_cli_unfocus(c);
  cli_deinit(c);
  cblk_unmap(&c->wk->clis, c);
}

cli_t*
wm_cli(Window const win) {
  wk_t* wk = wks.front;
  do {
    cli_t* const c = cli(win, wk);
    if (c) {
      return c;
    }

    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);
  return NULL;
}

cli_t*
wm_cli_move(cli_t* const c, wk_t* const wk) {
  /* Duplicate c in wk */
  cli_t* const nextc = cblk_map(&wk->clis, c);
  if (nextc) {
    nextc->wk = wk;
    if (wk->clis.size == 1) {
      wk->prevc = wk->currc = nextc;
    } else {
      wk->prevc = wk->currc;
      wk->currc = nextc;
    }

    XReparentWindow(dpy, nextc->ico.win, wk->wg.win, 0, 0);
    cli_t* const prevc = c->wk->clis.size == 1 ? NULL : 
      cblk_prev(&c->wk->clis, c);
    if (prevc) {
      wm_cli_switch(prevc);
    } else {
      wm_cli_unfocus(c);
      c->wk->prevc = c->wk->currc = NULL;
    }

    /* Unmap c */
    XUnmapWindow(dpy, c->par.win);
    cblk_unmap(&c->wk->clis, c);
  }

  return nextc;
}

typedef struct {
  int x;
  int y;
} wm_pair_t;

static wm_pair_t
wm_xform_snap(int const x, int const y, int const w,
  int const h) {
  int X = x;
  int Y = y;
  int const snap = 16;
  if (x > currmon->x - snap && x < currmon->x + snap) {
    X = currmon->x;
  } else {
    int const x0 = x + w;
    if (x0 > currmon->x1 - snap && x0 < currmon->x1 + snap) {
      X = currmon->x1 - w;
    }
  }

  if (y > currmon->y - snap && y < currmon->y + snap) {
    Y = currmon->y;
  } else {
    int const y0 = y + h;
    if (y0 > currmon->y1 - snap && y0 < currmon->y1 + snap) {
      Y = currmon->y1 - h;
    }
  }

  return (wm_pair_t) { .x = X, .y = Y };
}

void
wm_cli_translate(cli_t* const c, int const x_root, 
  int const y_root) {
  long const MOUSEMASK = 
    ButtonReleaseMask |
    PointerMotionMask;
  if (XGrabPointer(dpy, c->par.win, False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.move, CurrentTime) != GrabSuccess) {
    return;
  }

  XEvent xev;
  int nextx = c->par.x;
  int nexty = c->par.y;
  XRaiseWindow(dpy, c->hd1.win);
  void (*tray_map_cb)(cli_t* const) = NULL;
  root_query();
  int const par_d = root_stack_dist(c->par.win);
  int const tray_d = root_stack_dist(tray.wg.win);
  long const mask = ExposureMask | MOUSEMASK;
  do {
    XMaskEvent(dpy, mask, &xev);
    if (xev.type == MotionNotify) {
      nextx = c->par.x + xev.xmotion.x_root - x_root;
      nexty = c->par.y + xev.xmotion.y_root - y_root;
      
      mon_t* const mon = mon_currmon(nextx, nexty);
      if (mon) {
        currmon = mon;
      }

      unsigned const dh = c->par.h + 2 * c->par.bw;
      if (nexty + dh > currmon->h) {
        nexty = currmon->h - dh;
      } else {
        wm_pair_t const xy = wm_xform_snap(nextx, nexty, 
          c->par.w + 2 * c->par.bw, 
          c->par.h + 2 * c->par.bw);
        nextx = xy.x;
        nexty = xy.y;
      }
      
      XMoveWindow(dpy, c->par.win, nextx, nexty);
      cli_hd1_draw(c, nextx, nexty);
    
      if (tray_d < par_d && currmon == mons.front && 
          xev.xmotion.x_root > currmon->w &&
          xev.xmotion.x_root < currmon->w + tray.wg.w) {
        wg_win_bdrclr(tray.wg.win, wg_ACT);
        tray_map_cb = wm_tray_cli_map;
      } else {
        wg_win_bdrclr(tray.wg.win, wg_BG);
        tray_map_cb = NULL;
      }
    }

    else if (xev.type == Expose) {
      Window const win = xev.xexpose.window;
      cli_t* const c = wm_cli(win);
      if (c) {
        int const clr = wg_BG;
        cli_clr(c, clr);
        cli_ico_clr(c, clr);
      } else if (win == tray.wg.win) {
        tray_mascot_conf();
      }
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  cli_move(c, nextx, nexty, currmon->x1, currmon->y1);
  XLowerWindow(dpy, c->hd1.win);
  cli_clr(c, wg_ACT);
  wg_win_bdrclr(tray.wg.win, wg_BG);
  if (tray_map_cb) {
    tray_map_cb(c);
  }
}

void
wm_cli_resize(cli_t* const c) {
  long const MOUSEMASK = 
    ButtonReleaseMask |
    PointerMotionMask;
  if (XGrabPointer(dpy, c->par.win, False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.resize, CurrentTime) != GrabSuccess) {
    return;
  }
  
  XWarpPointer(dpy, None, c->par.win, 0, 0, 0, 0, 
    c->par.w, c->par.h);
  XEvent xev;
  int nextw = c->par.w;
  int nexth = c->par.h;
  XRaiseWindow(dpy, c->hd1.win);
  long const mask = ExposureMask | MOUSEMASK;
  do {
    XMaskEvent(dpy, mask, &xev);
    if (xev.type == MotionNotify) {
      /*
      int w = xev.xmotion.x_root;
      nextw = w < 1 ? 1 : w;
      int h = xev.xmotion.y_root;
      nexth = h < 1 ? 1 : h;
      if (currmon->x + nextw > currmon->x + currmon->w ||
          currmon->y + nexth > currmon->y + currmon->h) {
        nextw = currmon->w;
        nexth = currmon->h;
      }
      */
      int w = xev.xmotion.x_root;
      nextw = w - c->par.x - c->par.bw;
      nextw = nextw > currmon->x1 ? currmon->x1 : nextw;
      int h = xev.xmotion.y_root;
      nexth = h - c->par.y - c->par.bw;
      nexth = nexth > currmon->y1 ? currmon->y1 : nexth;

      XResizeWindow(dpy, c->ker.win, nextw, nexth - c->hd1.h);
      XResizeWindow(dpy, c->hd1.win, nextw, c->hd1.h);
      XResizeWindow(dpy, c->par.win, nextw, nexth);
      cli_hd1_draw(c, nextw, nexth);
    } else if (xev.type == Expose) {
      Window const win = xev.xexpose.window;
      cli_t* const c = wm_cli(win);
      if (c) {
        int const clr = wg_BG;
        cli_clr(c, clr);
        cli_ico_clr(c, clr);
      } else if (win == tray.wg.win) {
        tray_mascot_conf();
      }
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  XLowerWindow(dpy, c->hd1.win);
  wm_cli_ker_conf(c, nextw, nexth - c->hd1.h);
  cli_clr(c, wg_ACT);
}

void
wm_cli_del(cli_t* const c) {
  wk_t* const wk = c->wk;
  cli_t* const prevc = cblk_prev(&wk->clis, c);
  if (prevc != c) {
    wk->currc = prevc;
  } else {
    wk->prevc = wk->currc = NULL;
  }

  if (wk->prevc == c) {
    wk->prevc = wk->currc;
  }

  XUnmapWindow(dpy, c->par.win);
  cli_del_anim(c, 100);
  wm_cli_unmap(c);
  if (wk->currc) {
    wm_cli_focus(wk->currc);
  }
}

void
wm_cli_unfocus(cli_t* const c) {
  input_btns_ungrab(c->ker.win);
  XSetInputFocus(dpy, DefaultRootWindow(dpy), 
    RevertToPointerRoot, CurrentTime);

  cli_clr(c, wg_BG);
  cli_ico_clr(c, wg_BG);
  c->wk->currc = NULL;
}

void
wm_cli_focus(cli_t* const c) {
  input_btns_grab(c->ker.win);
  XSetInputFocus(dpy, c->ker.win, RevertToPointerRoot,
    CurrentTime);

  cli_clr(c, wg_ACT);
  cli_ico_clr(c, wg_ACT);
  XRaiseWindow(dpy, c->ico.win);
  mon_t* const mon = mon_currmon(c->par.x, c->par.y);
  if (mon) {
    currmon = mon;
  }
}

void
wm_cli_sel(cli_t* const c) {
  cli_clr(c, wg_SEL);
  cli_ico_clr(c, wg_SEL);
}

void
wm_cli_switch(cli_t* const c) {
  if (c->wk->currc) {
    wm_cli_unfocus(c->wk->currc);
    c->wk->prevc = c->wk->currc;
  } else {
    c->wk->prevc = c;
  }

  cli_switch_anim(c, 20);
  wm_cli_focus(c);
  c->wk->currc = c;
}

void
wm_cli_ker_conf(cli_t* const c, int const w, int const h) {
  if (c->ker.w != w || c->ker.h != h) {
    cli_ker_resize(c, w, h, currmon->w, currmon->h);
    c->fl.w = c->ker.w;
    c->fl.h = c->ker.h;
  }
}

void
wm_cli_arrange(cli_t* const c, int const x, int const y) {
  int const X = currmon->x + x;
  int const Y = currmon->y + y;
  if (c->par.x != X || c->par.y != Y) {
    cli_move(c, X < currmon->x ? currmon->x : X, 
      Y < currmon->y ? currmon->y : Y, 
        currmon->x1, currmon->y1);
    c->fl.x = c->par.x;
    c->fl.y = c->par.y;
  }
}

void wm_cli_currmon_move(void) {
  wk_t* wk = wks.front;
  do {
    if (wk->clis.size) {
      cli_t* c = wk->clis.front;
      do {
        mon_t* const back = mons.back;
        if (c->par.x > back->x + back->w ||
            c->par.y > back->y + back->h) {
          cli_move(c, back->x, back->y, 
            currmon->w, currmon->h);
        }

        c = cblk_next(&wk->clis, c);
      } while (c != wk->clis.front);
    }

    wk = cblk_next(&wks, wk);
  } while (wk != wks.front); 
}

void
wm_cli_min(cli_t* const c) {
  XUnmapWindow(dpy, c->par.win);
  cli_anim(c->par.x, c->par.y, c->par.w, c->par.h, 
    0, currmon->h, 1, 1, 100);
  c->mode = cli_MIN;
}

void
wm_cli_max(cli_t* const c) {
  XUnmapWindow(dpy, c->par.win);
  unsigned int bw = 0;
  XSetWindowBorderWidth(dpy, c->par.win, bw);
  cli_anim(c->par.x, c->par.y, c->par.w, c->par.h, 
    currmon->x, currmon->y, currmon->w, currmon->h, 100);
  c->mode = cli_MAX;
  XMoveResizeWindow(dpy, c->par.win, 
    currmon->x, currmon->y, currmon->w, currmon->h);
  int const w0 = c->ker.w;
  int const h0 = c->ker.h;
  cli_par_conf(c, currmon->w, currmon->h, bw);
  c->ker.w = w0;
  c->ker.h = h0;
  XMapWindow(dpy, c->par.win);
}

void
wm_cli_fs(cli_t* const c) {
  XUnmapWindow(dpy, c->par.win);
  XSetWindowBorderWidth(dpy, c->par.win, 0);
  cli_anim(c->ker.x, c->ker.y, c->ker.w, c->ker.h, 
    currmon->x, currmon->y, 
    currmon->w, currmon->h, 100);
  XMapWindow(dpy, c->par.win);
  XMoveResizeWindow(dpy, c->par.win, 
    currmon->x, currmon->y, currmon->w, currmon->h);
  XMoveResizeWindow(dpy, c->ker.win, 0, 0, 
    currmon->w, currmon->h);
  XRaiseWindow(dpy, c->ker.win);
}

void
wm_cli_res(cli_t* const c) {
  XUnmapWindow(dpy, c->par.win);
  XSetWindowBorderWidth(dpy, c->par.win, c->par.bw);
  cli_anim(currmon->x, currmon->y, 
    currmon->w, currmon->h, 
      c->par.x, c->par.y, c->ker.w, c->ker.h, 100);
  c->mode = cli_RES;
  cli_ker_conf(c, c->ker.w, c->ker.h);
  XMoveWindow(dpy, c->par.win, c->par.x, c->par.y);
  XMapWindow(dpy, c->par.win);
}

void
wm_cli_raise(cli_t* const c) {
  cli_anim(0, currmon->h, 0, 0, 
    c->par.x, c->par.y, c->ker.w, c->ker.h, 100);
  c->mode = cli_RES;
  XMapRaised(dpy, c->par.win);
}

void
wm_cli_ico_enum(cli_t* const c) {
  char str[8];
  size_t n = cblk_dist(&c->wk->clis, c) + 1;
  snprintf(str, sizeof str - 1, "%zu %s", n, c->strico);
  wg_str_set(&c->ico, str);
}

void
wm_ico_enum(wk_t* const wk) {
  if (wk->clis.size == 0) {
    return;
  }

  cli_t* c = wk->clis.front;
  size_t n = 0;
  do {
    char str[8];
    snprintf(str, sizeof str - 1, "%zu %s", 
      n + 1, c->strico);
    wg_str_set(&c->ico, str);
    n++;
    c = cblk_next(&wk->clis, c);
  } while (c != wk->clis.front);
}

void
wm_tray_cli_map(cli_t* const c) {
  cli_t* const prevc = c->wk->clis.size == 1 ? NULL : 
    cblk_prev(&c->wk->clis, c);
  if (prevc) {
    wm_cli_switch(prevc);
  } else {
    wm_cli_unfocus(c);
    c->wk->prevc = c->wk->currc = NULL;
  }
 
  wg_t ker = c->ker;
  wm_cli_unmap(c);
  cblk_unmap(&c->wk->clis, c);
  tray_cli_map(&ker);
}

void
wm_tray_cli_unmap(wg_t* const wg) {
  XEvent xev = {
    .xmaprequest = (XMapRequestEvent) {
      .type = MapRequest,
      .display = dpy,
      .send_event = False,
      .parent = DefaultRootWindow(dpy),
      .window = wg->win
    }
  };

  XResizeWindow(dpy, wg->win, wg->w, wg->h);
  XPutBackEvent(dpy, &xev);
  tray_cli_unmap(wg);
}

Window*
wm_ord_map(Window const win) {
  cli_ord_init(win);
  return cblk_map(&ord, &win);
}

void
wm_ord_unmap(Window* const win) {
  XDestroyWindow(dpy, *win);
  cblk_unmap(&ord, win);
}

Window*
wm_ord(Window const win) {
  if (ord.size == 0) {
    return NULL;
  }

  Window* c = ord.front;
  do {
    if (*c == win) {
      return c;
    }

    c = cblk_next(&ord, c);
  } while (c != ord.front);
  return NULL;
}

void
wm_ord_conf(Window const win, int const w, int const h) {
  XResizeWindow(dpy, win, w, h);
}

void
wm_ord_arrange(Window const win, int const x, int const y) {
  XMoveWindow(dpy, win, x, y);
}
