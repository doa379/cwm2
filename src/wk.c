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

void
wk_unfocus(wk_t* const wk) {
  for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++) {
    if (c == wk->currc)
      cli_unfocus(c);

    XUnmapWindow(dpy, c->par.win);
  }
  
  wk_wg_focus(wk, wg_BG);
}

void
wk_focus(wk_t* const wk) {
  if (wk->clis.size) {
    for (cli_t* c = wk->clis.beg; c != wk->clis.end; c++) {
      /* Disable any transient events within this op */
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, c->par.win, &wa) == 0)
        return;

      XSelectInput(dpy, c->par.win, 0);
      XMapWindow(dpy, c->par.win);
      XSelectInput(dpy, c->par.win, wa.your_event_mask);
    }

    cli_focus(wk->currc);
  }
  
  wk_wg_focus(wk, wg_ACT);
}

