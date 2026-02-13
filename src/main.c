#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>

#include "xerror.h"
#include "wm.h"
#include "cli.h"
#include "clr.h"
#include "font.h"
#include "root.h"
#include "ev.h"
#include "mon.h"
#include "sel.h"
#include "prop.h"
#include "tray.h"
#include "panel.h"
#include "../config.h"

volatile sig_atomic_t sig_status;
Display* dpy;

static void sig_handler(int sig) {
  sig_status = 1;
  fprintf(stdout, "\nSig.\n");
}

int main(int const ARGC, char const* ARGV[]) {
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    fprintf(stderr, "Failed to set sig handler\n");
    return -1;
  }

  dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "Failed to open display\n");
    return -1;
  }

  if (xerror_wmcheck() != 0) {
    XCloseDisplay(dpy);
    return -1;
  }

  if (root_wins_init() != 0) {
    XCloseDisplay(dpy);
    return -1;
  }

  root_query();
  root_ev_enqueue();
  if (font_init()       != 0 ||
      clr_init()        != 0 ||
      sel_init()        != 0 ||
      mon_mons_init()   != 0 ||
      wm_init(num_wks)  != 0 ||
      tray_init()       != 0) {
    XCloseDisplay(dpy);
    return -1;
  }
  
  prop_init();
  root_init();
  cli_wg_init();
  panel_init();
  panel_conf();
  tray_conf();
  ev_init();
  XSync(dpy, False);
  while (sig_status == 0) {
    ev_call();
  }

  cli_wg_deinit();
  root_deinit();
  wm_deinit();
  panel_deinit();
  clr_deinit();
  font_deinit();
  root_wins_deinit();
  XCloseDisplay(dpy);
  return 0;
}
