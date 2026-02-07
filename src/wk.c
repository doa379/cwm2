#include <X11/Xlib.h>

#include "font.h"
#include "wm.h"
#include "wk.h"
#include "cli.h"

extern Display* dpy;
extern font_t font;
extern unsigned const bw;

static size_t const NRES = 100;

wk_t
wk_init(void) {
  wg_t const wg = wg_init(DefaultRootWindow(dpy), 
    font.cw, font.ch, bw);
  long const BTNMASK = 
    EnterWindowMask |
    LeaveWindowMask |
    ButtonPressMask;
  XSelectInput(dpy, wg.win, BTNMASK);
  wg_win_bgclr(wg.win, wg_BG);
  wg_win_bdrclr(wg.win, wg_ACT);

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
wk_clr(wk_t* const wk, unsigned const clr) {
  wg_win_bgclr(wk->wg.win, clr);
}
