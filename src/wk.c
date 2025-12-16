#include <X11/Xlib.h>

#include "wm.h"
#include "wk.h"
#include "cli.h"

extern Display* dpy;
extern cblk_t wks;
extern wk_t* prevwk;
extern wk_t* currwk;

extern unsigned ch;
extern unsigned cw;
extern unsigned const bdrw;

static size_t const NRES = 100;

wk_t* wk_init(void) {
  wg_t const wg = wg_init(DefaultRootWindow(dpy), 0, bdrw, 
    cw, ch - 2 * bdrw, bdrw);
  wg_win_setbg(wg.win, wg_BG);
  wg_win_setbdr(wg.win, wg_ACT);

  wk_t const wk = {
    .clis = cblk_init(sizeof(cli_t), NRES),
    .prevc = NULL,
    .currc = NULL,
    .wg = wg
  };

  size_t const currwk_ = cblk_dist(&wks, currwk);
  wk_t* const nextwk = cblk_map(&wks, &wk);
  if (nextwk) {
    prevwk = cblk_itr(&wks, currwk_);
    currwk = nextwk;
    return currwk;
  }

  return NULL;
}

void wk_deinit(wk_t* wk) {
  wg_deinit(&wk->wg);
  cblk_deinit(&wk->clis);
  cblk_unmap(&wks, wk);
}

void wk_wg_focus(wg_t* const wg, unsigned const clr) {
  wg_win_setbg(wg->win, clr);
}

int wk_focus(wk_t* const wk) {
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

void wk_focus_all(void) {
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

int wk_unmap(wk_t* const wk) {
  if (wks.size == 1)
    return -1;

  size_t const wk_ = wk == cblk_back(&wks) ?
    wks.size - 2 : cblk_dist(&wks, wk);

  wk_t* nextwk = wk == wks.beg ? 
    cblk_next(&wks, wk) : cblk_prev(&wks, wk);

  size_t const nextwk_ = cblk_dist(&wks, nextwk);
  size_t const prevwk_ = cblk_dist(&wks, prevwk);
  

  if (wk->clis.size) {
    size_t const prevc = nextwk->clis.size == 0 ?
      cblk_dist(&wk->clis, wk->prevc) :
      cblk_dist(&nextwk->clis, nextwk->prevc);
    size_t const currc = nextwk->clis.size == 0 ?
      cblk_dist(&wk->clis, wk->currc) :
      cblk_dist(&nextwk->clis, nextwk->currc);

    do {
      cli_t* const c = wk->clis.beg;
      cli_t* const nextc = cblk_map(&nextwk->clis, c);
      if (nextc) {
        nextc->wk = nextwk;
        XReparentWindow(dpy, nextc->ico.win, 
            nextwk->wg.win, c->ico.x, c->ico.y);
        cblk_unmap(&wk->clis, c);
      }
    } while (wk->clis.size);

    nextwk->prevc = cblk_itr(&nextwk->clis, prevc);
    nextwk->currc = cblk_itr(&nextwk->clis, currc);
  }

  wk_deinit(wk);
  
  currwk = cblk_itr(&wks, wk_);
  prevwk = cblk_itr(&wks, prevwk_);
  
  if (wks.size == 1)
    prevwk = currwk;
  
  return 0;
}
