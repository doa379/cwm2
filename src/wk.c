#include <X11/Xlib.h>

#include "font.h"
#include "wm.h"
#include "wk.h"
#include "cli.h"

extern Display* dpy;
extern font_t font;
extern unsigned const bdrw;

static size_t const NRES = 100;

wk_t
wk_init(void) {
  wg_t const wg = wg_init(DefaultRootWindow(dpy), 0, 
      bdrw, font.cw, font.ch - 2 * bdrw, bdrw);
  wg_win_setbg(wg.win, wg_BG);
  wg_win_setbdr(wg.win, wg_ACT);

  return (wk_t) {
    .clis = cblk_init(sizeof(cli_t), NRES),
    .prevc = NULL,
    .currc = NULL,
    .wg = wg
  };
}

void
wk_deinit(wk_t* const wk) {
  wg_deinit(&wk->wg);
  cblk_deinit(&wk->clis);
}

void
wk_wg_focus(wk_t* const wk, unsigned const clr) {
  wg_win_setbg(wk->wg.win, clr);
}

int wk_unmap(wk_t* const wk) {
  /*
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

  wm_wk_unmap(wk);
  
  currwk = cblk_itr(&wks, wk_);
  prevwk = cblk_itr(&wks, prevwk_);
  
  if (wks.size == 1)
    prevwk = currwk;
  */
  return 0;
}
