#include <X11/Xlib.h>
#include <string.h>

#include "mon.h"
#include "wk.h"
#include "cli.h"
#include "arrange.h"
#include "prop.h"

extern Display* dpy;
extern cblk_t wks;
extern wk_t* prevwk;
extern wk_t* currwk;
extern cblk_t mons;

extern unsigned ch;
extern unsigned cw;
extern unsigned const bdrw;

static unsigned wkw;

wg_t status;
wg_t panel;
/*********************************/  
void panel_status_unfocus(void) {
  wg_win_setbg(status.win, BG);
  wg_str_draw(&status, BG, 0);
}

void panel_status_focus(void) {
  wg_win_setbg(status.win, ACT);
  wg_str_draw(&status, ACT, 0);
  if (XResizeWindow(dpy, status.win, status.str.ext, 
        status.h))
    status.w = status.str.ext;
}
/*********************************/  
void panel_init(void) {
  panel = wg_init(DefaultRootWindow(dpy), 0, 0, 
      1, ch, 0);
  wg_win_setbg(panel.win, BG);

  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    XReparentWindow(dpy, wk->wg.win, panel.win, 0, 0);

  wkw = cw;
  status = wg_init(panel.win, 0, 0, 1, ch, 0);
  static unsigned const STATUSMASK = 
    PropertyChangeMask |
    ExposureMask;
  XSelectInput(dpy, status.win, STATUSMASK);
  wg_win_setbg(status.win, BG);
  panel_status_focus();
}

void panel_deinit(void) {
  wg_deinit(&status);
  wg_deinit(&panel);
}

void panel_icos_arrange(void) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++)
    if (XResizeWindow(dpy, wk->wg.win, wk->wg.w0, wk->wg.h))
      wk->wg.w = wk->wg.w0;

  unsigned n = currwk->clis.size;
  if (n) {
    unsigned w = (wks.size - 1 + n) * wkw + status.w;
    while (w > panel.w) {
      n--;
      w = (wks.size - 1 + n) * wkw + status.w;
    }

    if (XResizeWindow(dpy, currwk->wg.win, 
          n * wkw, currwk->wg.h))
      currwk->wg.w = n * wkw;
  }

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

  for (cli_t* c = beg; c != end; c++) {
    if (c == currwk->currc) {
      wg_win_setbg(c->ico.win, ACT);
      wg_win_setbdr(c->ico.win, ACT);
      wg_str_draw(&c->ico, ACT, 0);
    } else {
      wg_win_setbg(c->ico.win, BG);
      wg_win_setbdr(c->ico.win, BG);
      wg_str_draw(&c->ico, BG, 0);
    }

    arrange_sel_map(&c->ico);
  }

  arrange_sel_adj(0);
}

void panel_arrange(void) {
  for (wk_t* wk = wks.beg; wk != wks.end; wk++) {
    if (wk == currwk)
      wk_wg_focus(&wk->wg, ACT);
    else {
      wk_wg_focus(&wk->wg, BG);
      if (wk->clis.size) {
        cli_t* const c = wk->currc;
        XMoveWindow(dpy, c->ico.win, 0, 0);
        wg_win_setbg(c->ico.win, BG);
        wg_win_setbdr(c->ico.win, BG);
        wg_str_draw(&c->ico, BG, 0);
      }
    }
    
    arrange_sel_map(&wk->wg);
  }

  mon_t const* mon = mons.beg;
  arrange_sel_map(&status);
  arrange_sel_adj(4);
}

void panel_conf(void) {
  mon_t const* mon = mons.beg;
  if (XResizeWindow(dpy, panel.win, mon->w, panel.h)) {
    panel.w = mon->w;
    if (XMoveWindow(dpy, panel.win, 0, mon->h - panel.h)) {
      panel.y = mon->h - panel.h;
    }
  }
}
