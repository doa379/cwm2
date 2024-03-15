#include <stdio.h>
#include <signal.h>
#include <Xlib.h>
#include <wm.h>
#include <util.h>

volatile sig_atomic_t sig_status;

static void sighandler(int sig) {
  sig_status = 1;
  intr_event();
  fprintf(stdout, "\nSig.\n");
}

int main(const int ARGC, const char* ARGV[]) {
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

  init_atoms();
  init_ewmh();
  init_wks();
  init_drawable();
  init_windows();
  static ev_t EV[] = {
    { .evfn = noop, .EVENT = NOOP },
    { .evfn = mapnotify, .EVENT = MAPNOTIFY },
    { .evfn = unmapnotify, .EVENT = UNMAPNOTIFY },
    //{ .evfn = clientmessage, .EVENT = CLIENTMESSAGE },
    { .evfn = configurenotify, .EVENT = CONFIGURENOTIFY },
    { .evfn = maprequest, .EVENT = MAPREQUEST },
    { .evfn = noop, .EVENT = CONFIGUREREQUEST },
    { .evfn = noop, .EVENT = MOTIONNOTIFY },
    { .evfn = keypress, .EVENT = KEYPRESS },
    { .evfn = btnpress, .EVENT = BTNPRESS },
    { .evfn = enternotify, .EVENT = ENTERNOTIFY },
    { .evfn = propertynotify, .EVENT = PROPERTYNOTIFY }
  };

  static ev_t MSGEV[] = {
    { .evfn = noop, .PROP = WM_PROTOCOLS },
    { .evfn = noop, .PROP = WM_NAME },
    { .evfn = noop, .PROP = WM_DELETE_WINDOW },
    { .evfn = noop, .PROP = WM_STATE },
    { .evfn = noop, .PROP = WM_TAKE_FOCUS },
    { .evfn = noop, .PROP = NET_SUPPORTED },
    { .evfn = noop, .PROP = NET_WM_STATE },
    { .evfn = noop, .PROP = NET_WM_NAME },
    { .evfn = noop, .PROP = NET_WM_WINDOW_OPACITY },
    { .evfn = noop, .PROP = NET_ACTIVE_WINDOW },
    { .evfn = noop, .PROP = NET_WM_STATE_FULLSCREEN },
    { .evfn = noop, .PROP = NET_WM_WINDOW_TYPE },
    { .evfn = noop, .PROP = NET_WM_WINDOW_TYPE_DIALOG },
    { .evfn = noop, .PROP = NET_CLIENT_LIST },
    { .evfn = noop, .PROP = NET_NUMBER_OF_DESKTOPS },
    { .evfn = noop, .PROP = NET_WM_DESKTOP },
    { .evfn = switch_wks, .PROP = NET_CURRENT_DESKTOP },
    { .evfn = noop, .PROP = NET_SHOWING_DESKTOP },
  };

  // Init return events
  for (size_t i = { 0 }; i < LEN(EV); i++)
    init_event(&EV[i]);
  for (size_t i = { 0 }; i < LEN(MSGEV); i++)
    init_msgevent(&MSGEV[i]);
  // Init internal events
  init_events();
  if (signal(SIGINT, sighandler) == SIG_ERR)
    sig_status = 1;

  while (sig_status == 0) {
    const ev_t* EV = event();
    EV->evfn(EV->DATA[0], EV->DATA[1], EV->DATA[2]);
  }

  // Cleanup
  deinit_drawable();
  deinit_wm();
  deinit_root();
  deinit_dpy();
  return 0;
}
