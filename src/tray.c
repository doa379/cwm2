#include <X11/Xlib.h>
#include <stdio.h>

#include "tray.h"
#include "mon.h"

extern Display* dpy;
extern cblk_t mons;
extern wg_t panel;
extern unsigned const trayw;

static size_t const NRES = 10;
tray_t tray;

int tray_init(void) {
  tray.clis = cblk_init(sizeof(Window), NRES);
  if (tray.clis.beg == NULL) {
    fprintf(stderr, "Failed to alloc tray\n");
    return -1;
  }

  tray.wg = wg_init(DefaultRootWindow(dpy), 
      0, 0, 1, 1, 0);
  wg_win_setbg(tray.wg.win, BG);
  return 0;
}

void tray_deinit(void) {
  wg_deinit(&tray.wg);
  cblk_deinit(&tray.clis);
}

void tray_conf(void) {
  wg_t* const wg = &tray.wg;
  mon_t const* mon = mons.beg;
  if (XResizeWindow(dpy, wg->win, trayw, 
        mon->h - panel.h)) {
    wg->w = trayw;
    wg->h = mon->h - panel.h;
    if (XMoveWindow(dpy, wg->win, mon->w - trayw, 0)) {
      wg->x = mon->w - trayw;
      wg->y = 0;
    }
  }
}
