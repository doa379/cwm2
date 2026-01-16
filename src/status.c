#include <X11/Xlib.h>

#include "status.h"
#include "font.h"
#include "wg.h"

extern Display* dpy;

extern font_t font;

wg_t status;

void
status_init(Window const win) {
  status = wg_init(win, 1, font.ch, 0);
  static long const STATUSMASK = 
    PropertyChangeMask |
    ExposureMask;
  XSelectInput(dpy, status.win, STATUSMASK);
  wg_win_bgset(status.win, wg_BG);
  status_focus(wg_ACT);
}

void
status_deinit(void) {
  wg_deinit(&status);
}

void
status_focus(unsigned const clr) {
  wg_win_bgset(status.win, clr);
  wg_str_draw(&status, clr, 0);
  wg_win_resize(&status, status.str.ext, status.h);
}

void
status_str_set(char const* str) {
  wg_str_set(&status, str);
}
