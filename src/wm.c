#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "wm.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "input.h"
#include "evcalls.h"
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
      if (c == wk->currc)
        input_btns_ungrab(c->par.win);

      XReparentWindow(dpy, c->win, 
          DefaultRootWindow(dpy), c->x0, c->y0);
      XMapWindow(dpy, c->win);
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
wm_wk_focus_all(void) {

}

cli_t*
wm_cli_map(wk_t* const wk, Window const win) {
  cli_t const c = cli_init(win, wk);
  return cblk_map(&wk->clis, &c);
}

void
wm_cli_unmap(cli_t* const c) {
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

void
wm_cli_translate(cli_t* const c, int const x_root, 
  int const y_root) {
  long const MOUSEMASK = 
    ButtonReleaseMask |
    PointerMotionMask;
  long const MASK = 
    ExposureMask;
  if (XGrabPointer(dpy, c->par.win, False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.move, CurrentTime) != GrabSuccess) {
    return;
  }

  XEvent xev;
  int const x0 = c->x0;
  int const y0 = c->y0;
  int nextx = x0;
  int nexty = y0;
  XUnmapWindow(dpy, c->hd0.win);
  XMapWindow(dpy, c->hd1.win);
  void (*tray_map_cb)(cli_t* const) = NULL;
  root_query();
  int const par_d = root_stack_dist(c->par.win);
  int const tray_d = root_stack_dist(tray.wg.win);
  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      nextx = x0 + xev.xmotion.x_root - x_root;
      nexty = y0 + xev.xmotion.y_root - y_root;
      unsigned const dh = c->par.h + 2 * c->par.bdrw;
      if (nexty + dh > currmon->h) {
        nexty = currmon->h - dh;
      }
      
      XMoveWindow(dpy, c->par.win, nextx, nexty);
      char str[16];
      snprintf(str, sizeof str - 1, "(%d, %d)", 
        nextx, nexty);
      wg_str_set(&c->hd1, str);
      unsigned hdr0 = 2 * c->par.bdrw;
      wg_str_draw(&c->hd1, wg_SEL, hdr0);
    
      if (tray_d < par_d && currmon == mons.front && 
          xev.xmotion.x_root > currmon->w - tray.wg.w &&
          xev.xmotion.x_root < currmon->w) {
        wg_win_bdrset(tray.wg.win, wg_ACT);
        tray_map_cb = wm_tray_cli_map;
      } else {
          wg_win_bdrset(tray.wg.win, wg_BG);
          tray_map_cb = NULL;
      }
    }

    else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
          
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  XUnmapWindow(dpy, c->hd1.win);
  XMapWindow(dpy, c->hd0.win);
  cli_move(c, nextx, nexty);
  cli_wg_focus(c, wg_ACT);
  wg_win_bdrset(tray.wg.win, wg_BG);
  if (tray_map_cb) {
    tray_map_cb(c);
  }
}

void
wm_cli_resize(cli_t* const c) {
  long const MOUSEMASK = 
    ButtonReleaseMask |
    PointerMotionMask;
  long const MASK = 
    ExposureMask;
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
  XUnmapWindow(dpy, c->hd0.win);
  XMapWindow(dpy, c->hd1.win);
  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int w = xev.xmotion.x;
      nextw = w < 1 ? 1 : w;
      int h = xev.xmotion.y;
      nexth = h < 1 ? 1 : h;
      if (currmon->x + nextw > currmon->w ||
          currmon->y + nexth > currmon->h) {
        nextw = currmon->w;
        nexth = currmon->h;
      }

      XResizeWindow(dpy, c->win, nextw, nexth - c->hd1.h);
      XResizeWindow(dpy, c->hd1.win, nextw, c->hd1.h);
      XResizeWindow(dpy, c->par.win, nextw, nexth);
      char str[16];
      snprintf(str, sizeof str - 1, "(%d, %d)", 
        nextw, nexth);
      wg_str_set(&c->hd1, str);
      unsigned hdr0 = 2 * c->par.bdrw;
      wg_str_draw(&c->hd1, wg_SEL, hdr0);
      
    } else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  XUnmapWindow(dpy, c->hd1.win);
  XMapWindow(dpy, c->hd0.win);
  cli_resize(c, nextw, nexth - c->hd1.h);
  cli_wg_focus(c, wg_ACT);
}

void
wm_cli_kill(cli_t* const c) {
  cli_t* const prevc = cblk_prev(&c->wk->clis, c);
  if (prevc && prevc != c) {
    wm_cli_focus(prevc);
    prevc->wk->currc = prevc;
  } else if (prevc) {
    c->wk->prevc = c->wk->currc = NULL;
  }
  
  XReparentWindow(dpy, c->win, DefaultRootWindow(dpy), 
      0, 0);
  XDestroyWindow(dpy, c->win);
  wm_cli_unmap(c);
}

void
wm_cli_unfocus(cli_t* const c) {
  cli_wg_focus(c, wg_BG);
  input_btns_ungrab(c->win);
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, 
    CurrentTime);
}

void
wm_cli_focus(cli_t* const c) {
  XRaiseWindow(dpy, c->ico.win);
  input_btns_grab(c->win);
  XSetInputFocus(dpy, c->win, RevertToPointerRoot,
    CurrentTime);
  cli_wg_focus(c, wg_ACT);
}

void
wm_cli_switch(cli_t* const c) {
  if (c->wk->currc) {
    wm_cli_unfocus(c->wk->currc);
    c->wk->prevc = c->wk->currc;
  } else {
    c->wk->prevc = c;
  }

  wm_cli_focus(c);
  c->wk->currc = c;
}

void
wm_wk_unfocus(wk_t* const wk) {
  if (wk->clis.size) {
    cli_t* c = wk->clis.front;
    do {
      if (c == wk->currc) {
        wm_cli_unfocus(c);
      }

      XUnmapWindow(dpy, c->par.win);
      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front); 
  }

  wk_wg_focus(wk, wg_BG);
}

void
wm_wk_focus(wk_t* const wk) {
  if (wk->clis.size) {
    cli_t* c = wk->clis.front; 
    do {
      XMapWindow(dpy, c->par.win);
      c = cblk_next(&wk->clis, c);
    } while (c != wk->clis.front);

    wm_cli_focus(wk->currc);
  }
  
  wk_wg_focus(wk, wg_ACT);
}

void
wm_wk_switch(wk_t* const wk) {
  wm_wk_unfocus(currwk);
  if (currwk->currc) {
    wm_cli_unfocus(currwk->currc);
  }

  prevwk = currwk;
  wm_wk_focus(wk);
  currwk = wk;
}

void
wm_cli_conf(cli_t* const c, int const w, int const h) {
  cli_resize(c, c->par.w + 2 * c->par.bdrw > currmon->w ? 
    currmon->w : w, 
      c->par.h + 2 * c->par.bdrw > currmon->h ? 
        currmon->h : h);
}

void
wm_cli_arrange(cli_t* const c, int const x, int const y) {
  int const nextx = 
    x + c->par.w + 2 * c->par.bdrw > currmon->w ? 
      currmon->w - c->par.w - 2 * c->par.bdrw : x;
  int const nexty = 
    y + c->par.h + 2 * c->par.bdrw > currmon->h ? 
      currmon->h - c->par.h - 2 * c->par.bdrw : y;
  cli_move(c, nextx, nexty);


  /*  
  cli_t* const prev = cblk_prev(&c->wk->clis, c);
  int const nextx = prev ? 
    prev->x0 + c->hd0.h + c->par.bdrw : x;
  int const nexty = prev ? 
    prev->y0 + c->hd0.h + c->par.bdrw : y;
  wm_cli_arrange(c, nextx, nexty);
  */

}

void wm_cli_currmon_move(void) {
  wk_t* wk = wks.front;
  do {
    if (wk->clis.size) {
      cli_t* c = wk->clis.front;
      do {
        mon_t* const back = mons.back;
        if (c->x0 > back->x + back->w ||
            c->y0 > back->y + back->h) {
          cli_move(c, back->x, back->y);
        }

        c = cblk_next(&wk->clis, c);
      } while (c != wk->clis.front);
    }

    wk = cblk_next(&wks, wk);
  } while (wk != wks.front); 
}

void
wm_cli_min(cli_t* const c) {
  wm_cli_unfocus(c);
  cli_anim(c, c->x0, c->y0, c->par.w, c->par.h, 
    0, currmon->h, 0, 0, 100);
  XUnmapWindow(dpy, c->par.win);
  c->mode = cli_MIN;
}

void
wm_cli_max(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, 0);
  int const w_org = c->w;
  int const h_org = c->h;
  c->mode = cli_MAX;
  XRaiseWindow(dpy, c->par.win);
  cli_anim(c, c->x0, c->y0, c->par.w, c->par.h, 
    currmon->x, currmon->y, currmon->w, currmon->h, 100);
  cli_resize(c, currmon->w, currmon->h);
  c->w = w_org;
  c->h = h_org;
  XMoveWindow(dpy, c->par.win, currmon->x, currmon->y);
}

void
wm_cli_res(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, c->par.bdrw);
  c->mode = cli_RES;
  cli_anim(c, currmon->x, currmon->y, currmon->w,
    currmon->h, c->x0, c->y0, c->w, c->h, 100);
  cli_conf(c, c->w, c->h);
  cli_move(c, c->x0, c->y0);
}

void
wm_cli_raise(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, c->par.bdrw);
  c->mode = cli_RES;
  cli_anim(c, 0, currmon->h, 0, 0, c->x0, c->y0, c->w, c->h, 
    100);
  cli_conf(c, c->w, c->h);
  cli_move(c, c->x0, c->y0);
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
  /* Determine required vertical offset */
  unsigned v0 = 0;
  if (tray.clis.size) {
    wg_t* wg = tray.clis.front;
    do {
      v0+= wg->h;
      wg = cblk_next(&tray.clis, wg);
    } while (wg != tray.clis.front);
  }
  /* Horizontal offset will be null */
  unsigned const h0 = 0;
  XReparentWindow(dpy, c->win, tray.wg.win, h0, v0);
  cli_t* const prevc = c->wk->clis.size == 1 ? NULL : 
    cblk_prev(&c->wk->clis, c);
  if (prevc) {
    wm_cli_switch(prevc);
  } else {
    wm_cli_unfocus(c);
    c->wk->prevc = c->wk->currc = NULL;
  }
 
  wg_t wg = {
    .win = c->win,
  };

  wg_t* const nextwg = cblk_map(&tray.clis, &wg);
  if (nextwg) {
    wg_win_bdrset(nextwg->win, wg_ACT);
    wg_win_resize(nextwg, tray.wg.w, 
      c->w / c->h * tray.wg.w);
    wm_cli_unmap(c);
    cblk_unmap(&c->wk->clis, c);
  }
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

  XPutBackEvent(dpy, &xev);
  cblk_unmap(&tray.clis, wg);
  /* Rearrange the remainders */
  unsigned v0 = 0;
  if (tray.clis.size) {
    wg_t* wg = tray.clis.front;
    do {
      XMoveWindow(dpy, wg->win, 0, v0);
      v0+= wg->h;
      wg = cblk_next(&tray.clis, wg);
    } while (wg != tray.clis.front);
  }
}

Window*
wm_ord_map(Window const win) {
  cli_ord_init(win);
  return cblk_map(&ord, &win);
}

void
wm_ord_unmap(Window* const win) {
  cblk_unmap(&ord, win);
  XDestroyWindow(dpy, *win);
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
