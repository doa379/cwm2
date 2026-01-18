#include <X11/Xlib.h>
#include <string.h>

#include "panel.h"
#include "status.h"
#include "font.h"
#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "arrange.h"
#include "prop.h"

extern Display* dpy;

extern cblk_t wks;
extern cblk_t mons;

extern wg_t status;
extern font_t font;
extern unsigned const bdrw;

static unsigned wkw;

wg_t panel;

void
panel_init(void) {
  panel = wg_init(DefaultRootWindow(dpy), 1, font.ch, 0);
  wg_win_bgset(panel.win, wg_BG);

  wk_t* wk = wks.front;
  do {
    XReparentWindow(dpy, wk->wg.win, panel.win, 0, 0);
    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);

  wkw = 1.25 * font.cw;
  status_init(panel.win);
  panel_icos_arrange(wks.front);
  panel_arrange(wks.front);
}

void
panel_deinit(void) {
  status_deinit();
  wg_deinit(&panel);
}

static void
panel_wk_conf(wk_t* const wk_) {
  /* Size the workspaces to available estate */

  /* if space is tight, scrollable wks
      if space is still tight, scrollable clis
        if space is still tight summarise wk/clis
  */

  unsigned n = 0;
  wk_t* wk = wks.front;
  do {
    n += wk->clis.size;
    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);

  unsigned w = n * wkw + status.w;
  do {
    wg_win_resize(&wk->wg, 
      wk->clis.size ? wkw * wk->clis.size : wkw, wk->wg.h);
    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);
}

static void
panel_icos_arrange_(wk_t* const wk) {
  wk_t* wk_ = wks.front;
  do {
    wg_win_resize(&wk_->wg, wkw, wk_->wg.h);
    wk_ = cblk_next(&wks, wk);
  } while (wk_ != wks.front);

  unsigned n = wk->clis.size;
  if (n) {
    unsigned w = (wks.size - 1 + n) * wkw + status.w;
    while (w > panel.w) {
      n--;
      w = (wks.size - 1 + n) * wkw + status.w;
    }

    wg_win_resize(&wk->wg, n * wkw, wk->wg.h);
  }

  cli_t* beg = wk->clis.front;
  cli_t const* end = wk->clis.back;
  if (n < wk->clis.size) {
    int d = cblk_dist(&wk->clis, wk->currc) - 0.5 * n;
    if (d < 0)
      d = 0;
    
    while (d + n > wk->clis.size)
      d--;

    beg = cblk_itr(&wk->clis, d);
    end = cblk_itr(&wk->clis, d + n + 1);
  }

  for (cli_t* c = beg; c != end; 
    c = cblk_next(&wk->clis, c))
    arrange_sel_map(&c->ico);

  arrange_sel_adj(0);
}

void
panel_icos_arrange(wk_t* const wk) {
  /* Update wk */
  if (wk->clis.size == 0)
    return;

  cli_t* c = wk->clis.front;
  do {
    arrange_sel_map(&c->ico);
    c = cblk_next(&wk->clis, c);
  } while (c != wk->clis.front);
  
  arrange_sel_adj(0);
}

void
panel_arrange(wk_t* const currwk) {
  panel_wk_conf(currwk);
  wk_t* wk = wks.front; 
  do {
    arrange_sel_map(&wk->wg);
    wk = cblk_next(&wks, wk);
  } while (wk != wks.front);

  arrange_sel_map(&status);
  arrange_sel_adj(4);
}

void
panel_icos_arrange_all(void) {

}

void
panel_arrange_all(void) {

}

void
panel_conf(void) {
  mon_t* const mon = mons.front;
  wg_win_resize(&panel, mon->w, panel.h);
  mon->h -= panel.h + 2 * panel.bdrw;
  XMoveWindow(dpy, panel.win, 0, mon->h);
  /*
  XMoveWindow(dpy, panel.win, 0, 
    mon->h - panel.h - 2 * panel.bdrw);
  */
}
