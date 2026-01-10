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

cblk_t wks;
wk_t* prevwk;
wk_t* currwk;
mon_t* currmon;

int
wm_init(unsigned const n) {
  wks = cblk_init(sizeof(wk_t), n > 0 ? n : 1);
  if (wks.blk == NULL) {
    fprintf(stderr, "Failed to alloc wm\n");
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
  wk_t* wk;
  while ((wk = cblk_back(&wks))) {
    /* Destroy from the end */
    cli_t* c;
    while ((c = cblk_back(&wk->clis))) {
      if (c == wk->currc)
        input_btns_ungrab(c->par.win);

      XReparentWindow(dpy, c->win, 
          DefaultRootWindow(dpy), c->x, c->y);
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
  if (wks.size == 1 ||
      wk->clis.size == 0)
    return -1;

  wk_t* const nextwk = cblk_next(&wks, wk);
  while (wk->clis.size) {
    cli_t* const c = wk->clis.front;
    wm_cli_move(c, nextwk);
  }

  if (prevwk == currwk)
    prevwk = nextwk;

  wm_wk_focus(nextwk);
  currwk = nextwk;
  wm_wk_deinit(wk);
  return 0;
}

void
wm_wk_focus_all(void) {

}

cli_t*
wm_cli_map(wk_t* const wk, Window const win, 
int const w, int const h) {
  static unsigned long const WINMASK = 
    FocusChangeMask | 
    StructureNotifyMask;
  static unsigned long const CLIMASK = 
    StructureNotifyMask |
    EnterWindowMask |
    ButtonPressMask |
    PropertyChangeMask;
  static unsigned long const HDRMASK =
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  static unsigned long const BTNMASK = 
    EnterWindowMask |
    LeaveWindowMask |
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  cli_t const c = cli_init(win, wk, w, h);
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
  wk_t* wk = wks.front;
  do {
    cli_t* const c = cli(win, wk);
    if (c)
      return c;

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
    if (wk->clis.size == 1)
      wk->prevc = wk->currc = nextc;
    else {
      wk->prevc = wk->currc;
      wk->currc = nextc;
    }

    XReparentWindow(dpy, nextc->ico.win, wk->wg.win, 0, 0);
    cli_t* const prevc = c->wk->clis.size == 1 ? NULL : 
      cblk_prev(&c->wk->clis, c);
    if (prevc)
      wm_cli_switch(prevc);
    else {
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
  static unsigned const MOUSEMASK = 
    ButtonReleaseMask |
    PointerMotionMask;
  static unsigned const MASK = 
    ExposureMask;
  if (XGrabPointer(dpy, DefaultRootWindow(dpy), False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.move, CurrentTime) != GrabSuccess)
    return;

  XEvent xev;
  int const x0 = c->x;
  int const y0 = c->y;
  void (*tray_map_cb)(cli_t* const) = NULL;
  
  root_query();
  int const par_d = root_stack_dist(c->par.win);
  int const tray_d = root_stack_dist(tray.wg.win);
  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int const nextx = x0 + xev.xmotion.x_root - x_root;
      int const nexty = y0 + xev.xmotion.y_root - y_root;
      cli_move(c, nextx, nexty);
      if (c->y + c->par.h + 2 * c->par.bdrw > currmon->h)
        cli_move(c, nextx, 
          currmon->h - c->par.h - 2 * c->par.bdrw);
    
      if (tray_d < par_d &&
        /* but why, par will be raised anyway */
          currmon == mons.front && 
          xev.xmotion.x > currmon->w - tray.wg.w &&
          xev.xmotion.x < currmon->w) {
        wg_win_bdrset(tray.wg.win, wg_SEL);
        tray_map_cb = tray_cli_map;
      } else {
          wg_win_bdrset(tray.wg.win, wg_BG);
          tray_map_cb = NULL;
      }
    }

    if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
          
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  wg_win_bdrset(tray.wg.win, wg_BG);
  if (tray_map_cb)
    tray_map_cb(c);
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
  int const x0 = c->x;
  int const y0 = c->y;
  XWarpPointer(dpy, None, c->par.win, 0, 0, 0, 0,
    c->x1, c->y1);
  do {
    XMaskEvent(dpy, MASK | MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int const dx = xev.xmotion.x - x0;
      int const dy = xev.xmotion.y - y0;
      int const bdrw = 2 * c->par.bdrw;
      if (xev.xmotion.x + x0 + bdrw < currmon->w &&
          xev.xmotion.y + y0 + bdrw < currmon->h)
        cli_conf(c, dx, dy);
    } else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
  } while (xev.type != ButtonRelease);
  XWarpPointer(dpy, None, c->par.win, 0, 0, 0, 0,
    c->x1, c->y1);
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
  /*
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
      CurrentTime);
  */
}

void
wm_cli_focus(cli_t* const c) {
  XMapRaised(dpy, c->par.win);
  XMapRaised(dpy, c->ico.win);
  input_btns_grab(c->par.win);
  /*
  XSetInputFocus(dpy, c->par.win, RevertToPointerRoot,
    CurrentTime);
  */
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
  if (wk->clis.size) {
    cli_t* c = wk->clis.front;
    do {
      if (c == wk->currc)
        wm_cli_unfocus(c);

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
      /* Disable any transient events within this op */
      XSelectInput(dpy, c->par.win, 0);
      XMapWindow(dpy, c->par.win);
      XSelectInput(dpy, c->par.win, c->par.mask);
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
  cli_conf(c, c->par.w + 2 * c->par.bdrw > currmon->w ? 
    currmon->w : w, 
    c->par.h + 2 * c->par.bdrw > currmon->h ? 
      currmon->h : h);
}

void
wm_cli_arrange(cli_t* const c, int const x, int const y) {
  int const x1 = c->x1;
  int const y1 = c->y1;
  int const nextx = x1 > currmon->w ? 0 : x;
  int const nexty = y1 > currmon->h ? 0 : y;
  cli_move(c, nextx, nexty);
}

void wm_cli_currmon_move(void) {
  wk_t* wk = wks.front;
  do {
    if (wk->clis.size) {
      cli_t* c = wk->clis.front;
      do {
        mon_t* const back = mons.back;
        if (c->x > back->x + back->w ||
            c->y > back->y + back->h) {
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
  cli_anim(c, c->x, c->y, c->par.w, c->par.h, 
    0, currmon->h, 0, 0, 10);
  XUnmapWindow(dpy, c->par.win);
  c->mode = MIN;
}

void
wm_cli_max(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, 0);
  int const w_org = c->w;
  int const h_org = c->h;
  c->mode = MAX;
  cli_anim(c, c->x, c->y, c->par.w, c->par.h, 
    currmon->x, currmon->y, currmon->w, currmon->h, 10);
  cli_resize(c, currmon->w, currmon->h);
  c->w = w_org;
  c->h = h_org;
  XMoveWindow(dpy, c->par.win, currmon->x, currmon->y);
}

void
wm_cli_res(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, c->par.bdrw);
  c->mode = RES;
  cli_anim(c, currmon->x, currmon->y, currmon->w,
    currmon->h, c->x, c->y, c->w, c->h, 10);
  cli_conf(c, c->w, c->h);
  cli_move(c, c->x, c->y);
}

void
wm_cli_raise(cli_t* const c) {
  XSetWindowBorderWidth(dpy, c->par.win, c->par.bdrw);
  c->mode = RES;
  cli_anim(c, 0, currmon->h, 0, 0, c->x, c->y, c->w, c->h, 
    10);
  cli_conf(c, c->w, c->h);
  cli_move(c, c->x, c->y);
}
