#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <ev.h>
#include <Xlib.h>

static const char* WMNAME = "cwm2";
static const char* WMVER = "-0.0";
volatile sig_atomic_t sig_status;

static void sighandler(int sig) {
  sig_status = 1;
  fprintf(stdout, "\n%s exit\n", WMNAME);
}

/*
static int XError(Display*, XErrorEvent* xev) {
  xerror = xev->error_code == BadAccess;
  return 0;
}
*/
int main(const int ARGC, const char* ARGV[]) {
  /*
  Display* dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "Failed to open display\n");
    return -1;
  }
  
  const Window ROOTW = XRootWindow(dpy, DefaultScreen(dpy));
  XSetErrorHandler(XError);
  XSelectInput(dpy, ROOTW, ROOTMASK);
  if (xerror) {
    XCloseDisplay(dpy);
    fprintf(stderr, "Initialization error (another wm running?)");
    return -1;
  }
  */

  if (!init_dpy()) {
    fprintf(stderr, "Failed to open display\n");
    return -1;
  } 

  if (!init_root()) {
    deinit_dpy();
    fprintf(stderr, "Initialization error (another wm running?)");
    return -1;
  }

  if (!init_wm()) {
    deinit_dpy();
    fprintf(stderr, "Initialization error (Failed to alloc for clients)");
    return -1;
  }

  init_panel();
  draw_root(WMNAME);
  init_windows();
  if (signal(SIGINT, sighandler) == SIG_ERR)
    sig_status = 1;
  
  static ev_t EV[] = {
    { .evfn = noop },
    { .evfn = mapnotify },
    { .evfn = unmapnotify },
    { .evfn = clientmessage },
    { .evfn = configurenotify },
    { .evfn = maprequest },
    { .evfn = keypress },
    { .evfn = btnpress },
    { .evfn = enternotify },
    { .evfn = propertynotify }
  };

  // Init return events
  init_noop(&EV[0]);
  init_mapnotify(&EV[1]);
  init_unmapnotify(&EV[2]);
  init_clientmessage(&EV[3]);
  init_configurenotify(&EV[4]);
  init_maprequest(&EV[5]);
  init_keypress(&EV[6]);
  init_btnpress(&EV[7]);
  init_enternotify(&EV[8]);
  init_propertynotify(&EV[9]);
  // Init internal events
  init_events();
  while (sig_status == 0) {
    const ev_t* EV = event();
    EV->evfn(EV->DATA[0], EV->DATA[1], EV->DATA[2]);
  }

  // Cleanup
  deinit_panel();
  deinit_wm();
  deinit_root();
  deinit_dpy();
  return 0;
}
