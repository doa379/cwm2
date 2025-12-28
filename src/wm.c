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

  wk_focus(wks.beg);
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
  static unsigned const WINMASK = FocusChangeMask;
  static unsigned const CLIMASK = StructureNotifyMask |
    EnterWindowMask |
    ButtonPressMask |
    PropertyChangeMask;
  static unsigned const HDRMASK = ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  static unsigned const BTNMASK = EnterWindowMask |
    LeaveWindowMask |
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  cli_t const c = cli_init(win, wk, mon, x, y, w, h);
  cli_t* const nextc = cblk_map(&wk->clis, &c);
  if (nextc) {
    XSelectInput(dpy, win, WINMASK);
    XSelectInput(dpy, nextc->par.win, CLIMASK);
    XSelectInput(dpy, nextc->hdr.win, HDRMASK);
    XSelectInput(dpy, nextc->min.win, BTNMASK);
    XSelectInput(dpy, nextc->max.win, BTNMASK);
    XSelectInput(dpy, nextc->cls.win, BTNMASK);
    XSelectInput(dpy, nextc->ico.win, BTNMASK);
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
  void (*map_cb)(void) = NULL;

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
        /* map_cb = ; */
      }
      else
        wg_win_bdrset(tray.wg.win, wg_BG);
    } else if (xev.type == Expose) {
        Window const win = xev.xexpose.window;
        evcalls_expose(win);
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
  wg_win_bdrset(tray.wg.win, wg_BG);
  if (map_cb)
    map_cb();
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
    cli_focus(prevc);
    prevc->wk->currc = prevc;
  } else if (prevc)
    c->wk->prevc = c->wk->currc = NULL;
  
  cli_kill(c);
}

void
wm_cli_switch(cli_t* const c) {
  if (c->wk->currc) {
    cli_unfocus(c->wk->currc);
    input_btns_ungrab(c->wk->currc->par.win);
    c->wk->prevc = c->wk->currc;
  } else
    c->wk->prevc = c;

  cli_focus(c);
  input_btns_grab(c->par.win);
  currwk->currc = c;
}

void
wm_wk_switch(wk_t* const wk) {
  wk_unfocus(currwk);
  if (currwk->currc) {
    cli_unfocus(currwk->currc);
    input_btns_ungrab(currwk->currc->par.win);
  }

  prevwk = currwk;
  wk_focus(wk);
  currwk = wk;
}
