#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "wm.h"
#include "wk.h"
#include "cli.h"
#include "input.h"
#include "font.h"

extern Display* dpy;
extern font_t font;

cblk_t wks;
wk_t* prevwk;
wk_t* currwk;

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

  prevwk = currwk = wks.beg;
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
  if (wks.size == 1)
    return -1;

  return 0;
}

int
wm_wk_focus(wk_t* const wk) {
  if (wk == currwk)
    return -1;

  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; 
        c != wk->clis.end; c++) {
      XUnmapWindow(dpy, c->par.win);
    }
  
  prevwk = currwk;
  currwk = wk;
  if (currwk->clis.size) {
    for (cli_t* c = currwk->clis.beg; 
        c != currwk->clis.end; c++)
      XMapWindow(dpy, c->par.win);

    XSync(dpy, True);
    cli_t* const c = wk->currc;
    wm_cli_focus(c);
  }

  return 0;
}

void
wm_wk_focus_all(void) {
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
}

cli_t*
wm_cli_map(wk_t* const wk, Window const win, int const x, 
int const y) {
  static unsigned const CLIMASK =
    StructureNotifyMask |
    /*FocusChangeMask |*/
    EnterWindowMask |
    LeaveWindowMask |
    PropertyChangeMask;
  static unsigned const HDRMASK =
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  static unsigned const BTNMASK =
    EnterWindowMask |
    LeaveWindowMask |
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  
  cli_t const c = cli_init(win, currwk);
  /* Ptrs will invalidate after mapping/realloc */
  int const currc = wk->clis.size ? 
    cblk_dist(&wk->clis, wk->currc) : -1;
  cli_t* const nextc = cblk_map(&wk->clis, &c);
  if (nextc) {
    XSelectInput(dpy, nextc->par.win, CLIMASK);
    XSelectInput(dpy, nextc->hdr.win, HDRMASK);
    XSelectInput(dpy, nextc->min.win, BTNMASK);
    XSelectInput(dpy, nextc->max.win, BTNMASK);
    XSelectInput(dpy, nextc->cls.win, BTNMASK);
    XSelectInput(dpy, nextc->ico.win, BTNMASK);
    XMapWindow(dpy, nextc->par.win);
    XMapWindow(dpy, nextc->win);
    wk->currc = currc > -1 ? 
      cblk_itr(&wk->clis, currc) : NULL;
    if (wk->currc) {
      int const x = wk->currc->par.x;
      int const y = wk->currc->par.y;
      cli_arrange(nextc, x, y);
    } else if (x && y) 
      cli_arrange(nextc, x, y);

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

void
wm_cli_focus(cli_t* const c) {
  if (currwk->currc) {
    input_btns_ungrab(currwk->currc->par.win);
    cli_wg_focus(currwk->currc, wg_BG);
  }
  
  XRaiseWindow(dpy, c->par.win);
  XRaiseWindow(dpy, c->ico.win);
  XSetInputFocus(dpy, c->par.win, RevertToPointerRoot,
    CurrentTime);
  input_btns_grab(c->par.win);
  cli_wg_focus(c, wg_ACT);
  currwk->prevc = currwk->currc ? currwk->currc : c;
  currwk->currc = c;
}

int
wm_cli_move(cli_t* const c, wk_t* const wk) {
  cli_t* const nextc = cblk_map(&wk->clis, c);
  if (nextc == NULL)
    return -1;
  
  nextc->wk = wk;
  wk->prevc = wk->currc;
  wk->currc = nextc;
  XReparentWindow(dpy, nextc->ico.win, wk->wg.win,
      c->ico.x, c->ico.y);
  if (wk->clis.size == 1)
    wk->prevc = wk->currc;
  /* Resolve remainder after move */
  cli_t* const prevc = 
    c->wk->clis.size == 1 ? NULL :
    c->wk->currc == cblk_back(&c->wk->clis) ?
      cblk_prev(&c->wk->clis, c) : c;
  if (prevc) {
    wm_cli_focus(prevc);
    c->wk->currc = prevc;
  } else
    c->wk->prevc = c->wk->currc = NULL;
  /* Unmap c */
  XUnmapWindow(dpy, c->par.win);
  cblk_unmap(&c->wk->clis, c);
  return 0;
}

void
wm_cli_kill(cli_t* const c) {
  wk_t* const wk = c->wk;
  cli_t* const nextc = cblk_prev(&wk->clis, c);
  cli_t* const prevc = wk->prevc;
  XReparentWindow(dpy, c->win, DefaultRootWindow(dpy), 
      0, 0);
  XDestroyWindow(dpy, c->win);
  wm_cli_focus(nextc);
  if (prevc != c)
    wk->prevc = prevc;

  cli_deinit(c);
}

void
wm_cli_translate(cli_t* const c) {
  fprintf(stdout, "Calling move on client\n");
  XEvent xev;
  static unsigned const MOUSEMASK = 
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask;
  if (XGrabPointer(dpy, DefaultRootWindow(dpy), False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.move, CurrentTime) != GrabSuccess)
    return;

  do {
    XMaskEvent(dpy, MOUSEMASK, &xev);
    if (xev.type == MotionNotify) {
      int const x = xev.xmotion.x;
      int const y = xev.xmotion.y;
      int const x_root = xev.xmotion.x_root;
      int const y_root = xev.xmotion.y_root;
      if (XMoveWindow(dpy, c->par.win, x_root, y_root)) {
        c->par.x0 = c->par.x;
        c->par.x = x_root;
        c->par.y0 = c->par.y;
        c->par.y = y_root;
      }
    }
  } while (xev.type != ButtonRelease);
  XUngrabPointer(dpy, CurrentTime);
}

void
wm_cli_resize(cli_t* const c) {
  fprintf(stdout, "Calling resize on client\n");
  static unsigned const MOUSEMASK = 
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask;
  if (XGrabPointer(dpy, DefaultRootWindow(dpy), False, 
        MOUSEMASK, GrabModeAsync, GrabModeAsync, None, 
          font.crs.resize, CurrentTime) != GrabSuccess)
    return;
 

  XUngrabPointer(dpy, CurrentTime);
}
