#include <X11/Xlib.h>
#include <stdio.h>

#include "tray.h"
#include "mon.h"

extern Display* dpy;
extern int const bdrw;
extern cblk_t mons;
extern wg_t panel;
extern unsigned const trayw;

static size_t const NRES = 10;
tray_t tray;

int
tray_init(void) {
  tray.clis = cblk_init(sizeof(Window), NRES);
  if (tray.clis.beg == NULL) {
    fprintf(stderr, "Failed to alloc tray\n");
    return -1;
  }

  tray.wg = wg_init(DefaultRootWindow(dpy), 
      0, 0, 1, 1, bdrw);
  wg_win_bgset(tray.wg.win, wg_BG);
  wg_win_bdrset(tray.wg.win, wg_BG);
  return 0;
}

void
tray_deinit(void) {
  wg_deinit(&tray.wg);
  cblk_deinit(&tray.clis);
}

void
tray_conf(void) {
  wg_t* const wg = &tray.wg;
  mon_t const* mon = mons.beg;
  if (wg_win_resize(wg, trayw, mon->h - 2 * wg->bdrw - 
      panel.h - 2 * panel.bdrw) == 0)
    if (wg_win_move(wg, mon->w - trayw - 2 * wg->bdrw, 0) == 
        0)
      ;
}
