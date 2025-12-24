#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>

#include "wm.h"
#include "cli.h"
#include "clr.h"
#include "font.h"
#include "root.h"
#include "ev.h"
#include "mon.h"
#include "arrange.h"
#include "prop.h"
#include "tray.h"
#include "panel.h"
#include "mascot.h"
#include "../config.h"

static int xerror;
volatile sig_atomic_t sig_status;
Display* dpy;

static int XError_handler(Display*, XErrorEvent* xev) {
  xerror = (xev->error_code == BadAccess ||
    xev->error_code == BadWindow);
  return 0;
}

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

  XSelectInput(dpy, DefaultRootWindow(dpy), 
      SubstructureRedirectMask);
  XSetErrorHandler(XError_handler);
  if (xerror) {
    fprintf(stderr, 
        "Init. error (another wm running?)\n");
    XCloseDisplay(dpy);
    return -1;
  }

  root_query();
  if (font_init()       != 0 ||
      clr_init()        != 0 ||
      arrange_init()    != 0 ||
      mon_mons_init(1)  != 0 ||
      wm_init(num_wks)  != 0 ||
      tray_init()       != 0) {
    XCloseDisplay(dpy);
    return -1;
  }

  root_init();
  mon_conf();
  tray_conf();
  cli_wg_init();
  panel_init();
  panel_conf();
  mascot_init();
  mascot_draw();
  ev_init();
  XSync(dpy, False);
  while (sig_status == 0)
    ev_call();

  mascot_deinit();
  root_deinit();
  wm_deinit();
  panel_deinit();
  clr_deinit();
  font_deinit();
  XCloseDisplay(dpy);
  return 0;
}
