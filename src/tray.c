#include <X11/Xlib.h>
#include <stdio.h>

#include "tray.h"
#include "mascot.h"
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
  if (tray.clis.blk == NULL) {
    fprintf(stderr, "Failed to alloc tray\n");
    return -1;
  }

  tray.wg = wg_init(DefaultRootWindow(dpy), 1, 1, bdrw);
  long const TRAYMASK = 
    ButtonPressMask |
    ButtonReleaseMask |
    PointerMotionMask |
    ExposureMask;

  XSelectInput(dpy, tray.wg.win, TRAYMASK);
  wg_win_bgset(tray.wg.win, wg_BG);
  wg_win_bdrset(tray.wg.win, wg_BG);
  mascot_init(&tray.wg);
  return 0;
}

void
tray_deinit(void) {
  wg_deinit(&tray.wg);
  cblk_deinit(&tray.clis);
}

void
tray_conf(void) {
  mon_t* const mon = mons.front;
  wg_win_resize(&tray.wg, trayw, 
    mon->h - 2 * tray.wg.bdrw);
  int const x = mon->w - tray.wg.w - 2 * tray.wg.bdrw;
  XMoveWindow(dpy, tray.wg.win, x, 0);
  tray_mascot_conf();
}

void
tray_mascot_conf(void) {
  mascot_draw(&tray.wg, 0, tray.wg.h);
}

void tray_cli_map(cli_t* const c) {

}
