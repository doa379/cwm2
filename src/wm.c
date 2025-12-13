#include <assert.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "wk.h"
#include "cli.h"

extern Display* dpy;

cblk_t wks;
wk_t* prevwk;
wk_t* currwk;

int wm_init(unsigned const n) {
  wks = cblk_init(sizeof(wk_t), n > 0 ? n : 1);
  if (wks.beg == NULL) {
    fprintf(stderr, "Failed to alloc wm\n");
    return -1;
  }

  for (unsigned i = 0; i < n; i++)
    wk_init();

  prevwk = currwk = wks.beg;
  return 0;
}

void wm_deinit(void) {
  wk_t* wk;
  while ((wk = cblk_back(&wks))) {
    /* Destroy from the end */
    cli_t* c;
    while ((c = cblk_back(&wk->clis))) {
      XUngrabButton(dpy, AnyButton, AnyModifier, 
          c->par.win);
      XReparentWindow(dpy, c->win, DefaultRootWindow(dpy), 
          c->par.x, c->par.y);
      XMapWindow(dpy, c->win);
      cli_deinit(c);
    }

    wk_deinit(wk);
  }
  
  cblk_deinit(&wks);
}

cli_t* wm_cli_map(Window const win, int const x, 
    int const y) {
  static unsigned const CLIMASK =
    SubstructureRedirectMask |
    SubstructureNotifyMask |
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask |
    EnterWindowMask |
    LeaveWindowMask |
    PropertyChangeMask;
  static unsigned const BTNMASK =
    EnterWindowMask |
    ButtonPressMask |
    ButtonReleaseMask |
    ExposureMask;
  
  cli_t const c = cli_init(win);
  /* Ptrs will invalidate after mapping/realloc */
  int const currc = currwk->clis.size ? 
    cblk_dist(&currwk->clis, currwk->currc) : -1;
  cli_t* const nextc = cblk_map(&currwk->clis, &c);
  if (nextc) {
    XSelectInput(dpy, nextc->par.win, CLIMASK);
    XSelectInput(dpy, nextc->hdr.win, BTNMASK);
    XSelectInput(dpy, nextc->min.win, BTNMASK);
    XSelectInput(dpy, nextc->max.win, BTNMASK);
    XSelectInput(dpy, nextc->cls.win, BTNMASK);
    XSelectInput(dpy, nextc->ico.win, BTNMASK);
    XMapWindow(dpy, nextc->par.win);
    XMapWindow(dpy, nextc->win);
    currwk->currc = currc > -1 ? 
      cblk_itr(&currwk->clis, currc) : NULL;
    if (currwk->currc) {
      int const x = currwk->currc->par.x;
      int const y = currwk->currc->par.y;
      cli_arrange(nextc, x, y);
    } else if (x && y) 
      cli_arrange(nextc, x, y);

    return nextc;
  }

  return NULL;
}

void wm_cli_focus(cli_t* const c) {
  cli_focus(c);
  XRaiseWindow(dpy, c->par.win);
  XRaiseWindow(dpy, c->ico.win);
  XSetInputFocus(dpy, c->par.win, RevertToPointerRoot,
    CurrentTime);
}

int wm_cli_move(int const n) {
  /* n == -2 --> prev
   * n == -1 --> next
   * n >= 0  --> index
   */
  cli_t* const currc = currwk->currc;
  if (n == 0 || n > (int) wks.size || 
      n - 1 == cblk_dist(&wks, currwk) ||
      currc == NULL)
    return -1;

  wk_t* const nextwk = n == -2 ? 
      cblk_prev(&wks, currwk) :
        n == -1 ? cblk_next(&wks, currwk) :
          cblk_itr(&wks, n - 1);
  cli_t* const c = cblk_map(&nextwk->clis, currc);
  if (c == NULL)
    return -1;
  
  c->wk = nextwk;
  nextwk->prevc = nextwk->currc;
  nextwk->currc = c;
  XReparentWindow(dpy, c->ico.win, nextwk->wg.win,
      currc->ico.x, currc->ico.y);
  if (nextwk->clis.size == 1)
    nextwk->prevc = nextwk->currc;
  
  cli_t* const nextc = 
    currwk->clis.size == 1 ? NULL :
    currc == cblk_back(&currwk->clis) ?
      cblk_prev(&c->wk->clis, currc) : currc;
  if (nextc) {
    wm_cli_focus(nextc);
    currwk->currc = nextc;
  } else
    currwk->prevc = currwk->currc = NULL;

  XUnmapWindow(dpy, currc->par.win);
  cblk_unmap(&currwk->clis, currc);
  return 0;
}

void wm_cli_kill(cli_t* const c) {
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
