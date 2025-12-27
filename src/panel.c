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
  panel = wg_init(DefaultRootWindow(dpy), 0, 0, 
      1, font.ch, 0);
  wg_win_setbg(panel.win, wg_BG);

  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    XReparentWindow(dpy, wk->wg.win, panel.win, 0, 0);

  wkw = font.cw;
  status_init(panel.win);
  panel_icos_arrange(wks.beg);
  panel_arrange(wks.beg);
}

void
panel_deinit(void) {
  status_deinit();
  wg_deinit(&panel);
}

static void
panel_icos_arrange_(wk_t* const wk) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    wg_win_resize(&wk->wg, wk->wg.w0, wk->wg.h);

  unsigned n = wk->clis.size;
  if (n) {
    unsigned w = (wks.size - 1 + n) * wkw + status.w;
    while (w > panel.w) {
      n--;
      w = (wks.size - 1 + n) * wkw + status.w;
    }

    wg_win_resize(&wk->wg, n * wkw, wk->wg.h);
  }

  cli_t* beg = wk->clis.beg;
  cli_t const* end = wk->clis.end;
  if (n < wk->clis.size) {
    int d = cblk_dist(&wk->clis, wk->currc) - 0.5 * n;
    if (d < 0)
      d = 0;
    
    while (d + n > wk->clis.size)
      d--;

    beg = cblk_itr(&wk->clis, d);
    end = cblk_itr(&wk->clis, d + n);
  }

  for (cli_t* c = beg; c != end; c++)
    arrange_sel_map(&c->ico);

  arrange_sel_adj(0);
}

void
panel_icos_arrange(wk_t* const wk) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    panel_icos_arrange_(wk);

}

void
panel_arrange(wk_t const* currwk) {
  /*
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    if (wk == currwk)
      wk_wg_focus(wk, wg_ACT);
    else {
      wk_wg_focus(wk, wg_BG);
      if (wk->clis.size) {
        cli_t* const c = wk->currc;
        XMoveWindow(dpy, c->ico.win, 0, 0);
      }
    }
    
    arrange_sel_map(&wk->wg);
  }
  */
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    arrange_sel_map(&wk->wg);
  }

  mon_t const* mon = mons.beg;
  arrange_sel_map(&status);
  arrange_sel_adj(4);
}

void
panel_icos_arrange_all(void) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    if (XResizeWindow(dpy, wk->wg.win, wk->wg.w0, wk->wg.h))
      wk->wg.w = wk->wg.w0;

  unsigned n = 0;
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    n += wk->clis.size;
  
  if (n) {
    unsigned w = n * wkw + status.w;
    while (w > panel.w) {
      n--;
      w = n * wkw + status.w;
    }

    if (XResizeWindow(dpy, ((wk_t*) wks.beg)->wg.win, 
          n * wkw, ((wk_t*) wks.beg)->wg.h))
      ((wk_t*) wks.beg)->wg.w = n * wkw;
  }


  /*
  cli_t* beg = currwk->clis.beg;
  cli_t const* end = currwk->clis.end;
  if (n < currwk->clis.size) {
    int d = cblk_dist(&currwk->clis, currwk->currc) - 
      0.5 * n;
    if (d < 0)
      d = 0;
    
    while (d + n > currwk->clis.size)
      d--;

    beg = cblk_itr(&currwk->clis, d);
    end = cblk_itr(&currwk->clis, d + n);
  }
  */


  /*
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++) {
      if (c == currwk->currc) {
        wg_win_setbg(c->ico.win, wg_ACT);
        wg_win_setbdr(c->ico.win, wg_ACT);
        wg_str_draw(&c->ico, wg_ACT, 0);
      } else {
        wg_win_setbg(c->ico.win, wg_BG);
        wg_win_setbdr(c->ico.win, wg_BG);
        wg_str_draw(&c->ico, wg_BG, 0);
      }

      arrange_sel_map(&c->ico);
    }

  arrange_sel_adj(0);
  */
}

void
panel_arrange_all(void) {
  /*
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    if (wk == currwk)
      wk_wg_focus(wk, wg_ACT);
    else {
      wk_wg_focus(wk, wg_BG);
      if (wk->clis.size) {
        cli_t* const c = wk->currc;
        XMoveWindow(dpy, c->ico.win, 0, 0);
        wg_win_setbg(c->ico.win, wg_BG);
        wg_win_setbdr(c->ico.win, wg_BG);
        wg_str_draw(&c->ico, wg_BG, 0);
      }
    }
    
    arrange_sel_map(&wk->wg);
  }
  */

/*
  arrange_sel_map(&((wk_t*) wks.beg)->wg);
  mon_t const* mon = mons.beg;
  arrange_sel_map(&status);
  arrange_sel_adj(1);
*/
}

void
panel_conf(void) {
  mon_t const* mon = mons.beg;
  if (XResizeWindow(dpy, panel.win, mon->w, panel.h)) {
    panel.w = mon->w;
    if (XMoveWindow(dpy, panel.win, 0, mon->h - panel.h)) {
      panel.y = mon->h - panel.h;
    }
  }
}
