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
  tray.clis = cblk_init(sizeof(wg_t), NRES);
  if (tray.clis.blk == NULL) {
    fprintf(stderr, "Failed to alloc tray\n");
    return -1;
  }

  tray.wg = wg_init(DefaultRootWindow(dpy), 1, 1, bdrw);
  long const TRAYMASK =
    ExposureMask;
  XSelectInput(dpy, tray.wg.win, TRAYMASK);
  wg_win_bgclr(tray.wg.win, wg_BG);
  wg_win_bdrclr(tray.wg.win, wg_BG);
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

wg_t* 
tray_cli(Window const win) {
  if (tray.clis.size == 0) {
    return NULL;
  }

  wg_t* wg = tray.clis.front;
  do {
    if (wg->win == win) {
      return wg;
    }

    wg = cblk_next(&tray.clis, wg);
  } while (wg != tray.clis.front);
  return NULL;
}

unsigned
tray_cli_vd(void) {
  unsigned d = 0;
  if (tray.clis.size) {
    wg_t* wg = tray.clis.front;
    do {
      d += wg->h;
      wg = cblk_next(&tray.clis, wg);
    } while (wg != tray.clis.front);
  }

  return d;
}

void
tray_cli_map(wg_t* const wg) {
  unsigned const vd = tray_cli_vd();
  wg_t* const nextwg = cblk_map(&tray.clis, wg);
  if (nextwg) {
    XReparentWindow(dpy, nextwg->win, tray.wg.win, 
      -tray.wg.bdrw, vd);
    long const KMASK =
      ButtonPressMask |
      EnterWindowMask |
      LeaveWindowMask |
      SubstructureNotifyMask |
      SubstructureRedirectMask;
    XSelectInput(dpy, nextwg->win, KMASK);
    XSetWindowBorderWidth(dpy, nextwg->win, tray.wg.bdrw);
    wg_win_bdrclr(nextwg->win, wg_BG);
    wg_win_resize(nextwg, tray.wg.w, 
      tray.wg.w * (float) wg->h / wg->w);
  }
}
  
void
tray_cli_unmap(wg_t* const wg) {
  cblk_unmap(&tray.clis, wg);
  /* Rearrange the remainders */
  unsigned d = 0;
  if (tray.clis.size) {
    wg_t* wg = tray.clis.front;
    do {
      XMoveWindow(dpy, wg->win, -tray.wg.bdrw, d);
      d += wg->h;
      wg = cblk_next(&tray.clis, wg);
    } while (wg != tray.clis.front);
  }
}
