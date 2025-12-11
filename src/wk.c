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

/*********************************/  
void wk_wg_unfocus(wg_t* const wg) {
  wg_win_setbg(wg->win, BG);
}

void wk_wg_focus(wg_t* const wg) {
  wg_win_setbg(wg->win, ACT);
}
/*********************************/  
wk_t* wk_init(void) {
  wg_t const wg = wg_init(DefaultRootWindow(dpy), 0, bdrw, 
    cw, ch - 2 * bdrw, bdrw);
  wg_win_setbg(wg.win, BG);
  wg_win_setbdr(wg.win, ACT);

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

static void wk_focus(char const n) {
  for (cli_t* c = currwk->clis.beg; 
      c != currwk->clis.end; c++)
    XUnmapWindow(dpy, c->par.win);
  
  prevwk = currwk;
  currwk = n == -2 ? cblk_prev(&wks, currwk) :
    n == -1 ? cblk_next(&wks, currwk) :
    cblk_itr(&wks, n - 1);
  if (currwk->clis.size) {
    for (cli_t* c = currwk->clis.beg; 
        c != currwk->clis.end; c++)
      XMapWindow(dpy, c->par.win);

    cli_t* const c = currwk->currc;
    wm_cli_focus(c);
  }
}

int wk_unmap(void) {
  if (wks.size == 1)
    return -1;

  wk_t* nextwk = currwk == wks.beg ? 
    cblk_next(&wks, currwk) : cblk_prev(&wks, currwk);
  if (currwk->clis.size) {
    size_t const prevc = nextwk->clis.size == 0 ?
      cblk_dist(&currwk->clis, currwk->prevc) :
      cblk_dist(&nextwk->clis, nextwk->prevc);
    size_t const currc = nextwk->clis.size == 0 ?
      cblk_dist(&currwk->clis, currwk->currc) :
      cblk_dist(&nextwk->clis, nextwk->currc);
    while (currwk->clis.size) {
      cli_t* const c = currwk->clis.beg;
      cli_t* const nextc = cblk_map(&nextwk->clis, c);
      if (nextc) {
        nextc->wk = nextwk;
        XReparentWindow(dpy, nextc->ico.win, 
            nextwk->wg.win, c->ico.x, c->ico.y);
        cblk_unmap(&currwk->clis, c);
      }
    }

    nextwk->prevc = cblk_itr(&nextwk->clis, prevc);
    nextwk->currc = cblk_itr(&nextwk->clis, currc);
  }

  wk_deinit(currwk);
  currwk = nextwk;
  if (wks.size == 1)
    prevwk = currwk;

  return 0;
}

int wk_switch(int const n) {
  /* n == -2 --> prev
   * n == -1 --> next
   * n >= 0  --> index
   */
  if (n == 0 || n > (int) wks.size || 
      n - 1 == cblk_dist(&wks, currwk))
    return -1;

  wk_focus(n);
  return 0;
}
