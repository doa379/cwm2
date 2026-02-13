#include <X11/Xlib.h>

#include "status.h"
#include "font.h"
#include "wg.h"

extern Display* dpy;

extern font_t font;
extern unsigned const bw;

wg_t status;
wg_t mon;

static char const MON_INIT[] = "Mon 1";
static long const MASK = 
  ExposureMask;

void
status_init(Window const win) {
  mon = wg_init(win, 1, font.ch + 2 * bw, 0);
  wg_win_bgclr(mon.win, wg_BG);
  status_mon_str_set(MON_INIT);
  XSelectInput(dpy, mon.win, MASK);

  status = wg_init(win, 1, font.ch + 2 * bw, 0);
  wg_win_bgclr(status.win, wg_BG);
  XSelectInput(dpy, status.win, MASK);
}

void
status_deinit(void) {
  wg_deinit(&status);
  wg_deinit(&mon);
}

void
status_draw(unsigned const clr) {
  wg_win_bgclr(status.win, clr);
  wg_str_draw(&status, clr, 0);
  wg_win_resize(&status, status.str.ext, status.h);
}

void
status_str_set(char const* str) {
  wg_str_set(&status, str);
}

void
status_mon_str_set(char const* str) {
  wg_str_set(&mon, str);
}

void
status_mon_draw(unsigned const clr) {
  wg_win_bgclr(mon.win, clr);
  wg_str_draw(&mon, clr, 0);
  wg_win_resize(&mon, mon.str.ext, mon.h);
}
