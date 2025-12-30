#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "wm.h"
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
extern wg_t panel;
extern tray_t tray;

cblk_t wks;
wk_t* prevwk;
wk_t* currwk;
mon_t* currmon;

int
wm_init(unsigned const n) {
  wks = cblk_init(sizeof(wk_t), n > 0 ? n : 1);
  if (wks.beg == NULL) {
    fprintf(stderr, "Failed to alloc wm\n");
    return -1;
  }

  for (unsigned i = 0; i < n; i++)
    if (wm_wk_map() == NULL) {
      fprintf(stderr, "Failed to alloc wk\n");
      return -1;
    }

  wm_wk_focus(wks.beg);
  prevwk = currwk = wks.beg;
  currmon = mons.beg;
  return 0;
}

void
wm_deinit(void) {
  wk_t* wk;
  while ((wk = cblk_back(&wks))) {
    /* Destroy from the end */
    cli_t* c;
    while ((c = cblk_back(&wk->clis))) {
      if (c == wk->currc)
        input_btns_ungrab(c->par.win);

      XReparentWindow(dpy, c->win, 
          DefaultRootWindow(dpy), c->par.x, c->par.y);
      XMapWindow(dpy, c->win);
      cli_deinit(c);
    }

    wm_wk_deinit(wk);
  }
  
  cblk_deinit(&wks);
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
  /*
  if (wks.size == 1)
    return -1;
  */

  return 0;
}

void
wm_wk_focus_all(void) {
  /*
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++)
      XMapWindow(dpy, c->par.win);
   
  if (currwk->clis.size) {
    cli_t* const c = currwk->currc;
    wm_cli_focus(c);
  } else if (prevwk->clis.size) {
    cli_t* const c = prevwk->currc;
    wm_cli_focus(c);
  } else {
    for (wk_t* wk = wks.beg; wk != wks.end; wk++)
      if (wk->clis.size) {
        cli_t* const c = wk->currc;
        wm_cli_focus(c);
      }
  }
  */
}

cli_t*
wm_cli_map(mon_t* mon, wk_t* const wk, Window const win, 
int const x, int const y, int const w, int const h) {
  static unsigned long const WINMASK = FocusChangeMask;
  static unsigned long const CLIMASK = StructureNotifyMask |
    EnterWindowMask |
    ButtonPressMask |
    PropertyChangeMask;
  static unsigned long const HDRMASK = ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  static unsigned long const BTNMASK = EnterWindowMask |
    LeaveWindowMask |
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  cli_t const c = cli_init(win, wk, mon, x, y, w, h);
  cli_t* const nextc = cblk_map(&wk->clis, &c);
  if (nextc) {
    XSelectInput(dpy, win, WINMASK);
    nextc->mask = WINMASK;
    XSelectInput(dpy, nextc->par.win, CLIMASK);
    nextc->par.mask = CLIMASK;
    XSelectInput(dpy, nextc->hdr.win, HDRMASK);
    nextc->hdr.mask = HDRMASK;
    XSelectInput(dpy, nextc->min.win, BTNMASK);
    nextc->min.mask = BTNMASK;
    XSelectInput(dpy, nextc->max.win, BTNMASK);
    nextc->max.mask = BTNMASK;
    XSelectInput(dpy, nextc->res.win, BTNMASK);
    nextc->res.mask = BTNMASK;
    XSelectInput(dpy, nextc->cls.win, BTNMASK);
    nextc->cls.mask = BTNMASK;
    XSelectInput(dpy, nextc->ico.win, BTNMASK);
    nextc->ico.mask = BTNMASK;
    XMapWindow(dpy, nextc->par.win);
    XMapWindow(dpy, nextc->win);
    return nextc;
  }

  return NULL;
}

cli_t*
wm_cli(Window const win) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    cli_t* const c = cli(win, wk);
    if (c)
      return c;
  }

  return NULL;
}

cli_t*
wm_cli_move(cli_t* const c, wk_t* const wk) {
  /* Duplicate c in wk */
  cli_t* const nextc = cblk_map(&wk->clis, c);
  if (nextc) {
    nextc->wk = wk;
    if (wk->clis.size == 1)
      wk->prevc = wk->currc = nextc;
    else {
      wk->prevc = wk->currc;
      wk->currc = nextc;
    }

    XReparentWindow(dpy, nextc->ico.win, wk->wg.win,
      c->ico.x, c->ico.y);

    /* Resolve remainder after copy */
    cli_t* const prevc = c->wk->clis.size == 1 ? NULL : 
      cblk_prev(&c->wk->clis, c);
    /*
    wm_cli_unfocus(c);
    if (prevc) {
      wm_cli_focus(prevc);
    }
    */
    wm_cli_switch(prevc);

    /* Unmap c */
    XUnmapWindow(dpy, c->par.win);
    cblk_unmap(&c->wk->clis, c);
  }

  return nextc;
}

void
wm_cli_translate(cli_t* const c) {
  static unsigned const MOUSEMASK = 
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask;
  static unsigned const MASK = ExposureMask;
  if (XGrabPointer(dpy, DefaultRootWindow(dpy), False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.move, CurrentTime) != GrabSuccess)
    return;

  XEvent xev;
  int x;
  int y;
  if (root_ptr(&x, &y) == 0)
    return;
  
  int const x0 = c->par.x;
  int const y0 = c->par.y;
  int const x1 = currmon == mons.beg ? 
    currmon->w - tray.wg.w : currmon->w;
  int const y1 = currmon == mons.beg ? 
    currmon->h - panel.h : currmon->h;
  void (*map_cb)(cli_t* const) = NULL;

  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int const nextx = x0 + xev.xmotion.x - x;
      int const nexty = y0 + xev.xmotion.y - y;
      if (wg_win_move(&c->par, nextx, nexty) != 0)
        break;
      if (c->par.y + c->par.h > y1 - 2 * c->par.bdrw) {
        if (wg_win_move(&c->par, nextx, 
            y1 - c->par.h - 2 * c->par.bdrw) != 0)
          break;
      }
      
      if (xev.xmotion.x > x1) {
        wg_win_bdrset(tray.wg.win, wg_SEL);
        /* TODO tray action */
        map_cb = NULL;
      } else {
        wg_win_bdrset(tray.wg.win, wg_BG);
        map_cb = NULL;
      }
    } else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  wg_win_bdrset(tray.wg.win, wg_BG);
  if (map_cb)
    map_cb(c);
}

void
wm_cli_resize(cli_t* const c) {
  static unsigned const MOUSEMASK = 
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask;
  static unsigned const MASK = ExposureMask;
  if (XGrabPointer(dpy, DefaultRootWindow(dpy), False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.resize, CurrentTime) != GrabSuccess)
    return;
 
  XEvent xev;
  int const x0 = c->par.x;
  int const y0 = c->par.y;

  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int const w = xev.xmotion.x - x0;
      int const h = xev.xmotion.y - y0;
      if (wg_win_resize(&c->par, w, h) == 0)
        cli_conf(c, w, h);
      else
        break;
    } else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
}

void
wm_cli_kill(cli_t* const c) {
  cli_t* const prevc = cblk_prev(&c->wk->clis, c);
  if (prevc && prevc != c) {
    wm_cli_focus(prevc);
    prevc->wk->currc = prevc;
  } else if (prevc)
    c->wk->prevc = c->wk->currc = NULL;
  
  XReparentWindow(dpy, c->win, DefaultRootWindow(dpy), 
      0, 0);
  XDestroyWindow(dpy, c->win);
  cli_deinit(c);
}

void
wm_cli_unfocus(cli_t* const c) {
  cli_wg_focus(c, wg_BG);
  input_btns_ungrab(c->par.win);
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
      CurrentTime);
}

void
wm_cli_focus(cli_t* const c) {
  XMapRaised(dpy, c->par.win);
  XMapRaised(dpy, c->ico.win);
  input_btns_grab(c->par.win);
  XSetInputFocus(dpy, c->par.win, RevertToPointerRoot,
    CurrentTime);
  cli_wg_focus(c, wg_ACT);
}

void
wm_cli_switch(cli_t* const c) {
  if (c->wk->currc) {
    wm_cli_unfocus(c->wk->currc);
    c->wk->prevc = c->wk->currc;
  } else
    c->wk->prevc = c;

  wm_cli_focus(c);
  c->wk->currc = c;
}

void
wm_wk_unfocus(wk_t* const wk) {
  for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++) {
    if (c == wk->currc)
      wm_cli_unfocus(c);

    XUnmapWindow(dpy, c->par.win);
  }
  
  wk_wg_focus(wk, wg_BG);
}

void
wm_wk_focus(wk_t* const wk) {
  if (wk->clis.size) {
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++) {
      /* Disable any transient events within this op */
      XSelectInput(dpy, c->par.win, 0);
      XMapWindow(dpy, c->par.win);
      XSelectInput(dpy, c->par.win, c->par.mask);
    }

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
  cli_conf(c, w, h);
  int const w_ = currmon == mons.beg ? 
    currmon->w - tray.wg.w - 2 * tray.wg.bdrw : currmon->w;
  int const h_ = currmon == mons.beg ? 
    currmon->h - panel.h - 2 * panel.bdrw : currmon->h;
  cli_conf(c, c->par.w + 2 * c->par.bdrw > w_ ? w_ : w, 
    c->par.h + 2 * c->par.bdrw > h_ ? h_ : h);
}

void
wm_cli_arrange(cli_t* const c, int const x, int const y) {
  cli_arrange(c, x, y);
  int const x1 = c->par.x1;
  int const y1 = c->par.y1;
  int const x_ = currmon == mons.beg && 
    x1 > currmon->w - tray.wg.w - 2 * tray.wg.bdrw ? 0 : 
      x1 > currmon->w ? 0 :
        x;
  int const y_ = currmon == mons.beg && 
    y1 > currmon->h - panel.h - 2 * panel.bdrw ? 0 : 
      y1 > currmon->h ? 0 :
        y;

  cli_arrange(c, x_, y_);
}

void wm_cli_currmon_move(void) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++)
      if (c->mon >= (mon_t*) mons.end) {
        mon_t const* mon = cblk_back(&mons);
        wg_win_move(&c->par, mon->x, mon->y);
        c->mon = currmon;
      }
}

void
wm_cli_min(cli_t* const c) {
  wm_cli_unfocus(c);
  XUnmapWindow(dpy, c->par.win);
}

void
wm_cli_max(cli_t* const c) {
  int const w = currmon == mons.beg ? 
    currmon->w - tray.wg.w - 2 * tray.wg.bdrw :
    currmon->w;
  int const h = currmon == mons.beg ? 
    currmon->h - panel.h - 2 * panel.bdrw :
    currmon->h;
  c->mode = MAX;

  for (int i = 0; i < 100; i++)
    wm_cli_conf(c, w * i / 100., h * i / 100.);

  wm_cli_arrange(c, currmon->x, currmon->y);
}

void
wm_cli_res(cli_t* const c) {
  //wm_cli_switch(c);
  c->mode = RES;
  wm_cli_conf(c, c->par.w0, c->par.h0);
}
