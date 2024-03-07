#include <stdio.h>
#include <signal.h>
#include <wm.h>
#include <ev.h>
#include <Xlib.h>
#include <util.h>

volatile sig_atomic_t sig_status;

static void sighandler(int sig) {
  sig_status = 1;
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
  init_panel();
  init_windows();
  if (signal(SIGINT, sighandler) == SIG_ERR)
    sig_status = 1;

  static ev_t EV[] = {
    { .evfn = noop, .name = NOOP },
    { .evfn = mapnotify, .name = MAPNOTIFY },
    { .evfn = unmapnotify, .name = UNMAPNOTIFY },
    { .evfn = clientmessage, .name = CLIENTMESSAGE },
    { .evfn = configurenotify, .name = CONFIGURENOTIFY },
    { .evfn = maprequest, .name = MAPREQUEST },
    { .evfn = noop, .name = CONFIGUREREQUEST },
    { .evfn = noop, .name = MOTIONNOTIFY },
    { .evfn = keypress, .name = KEYPRESS },
    { .evfn = btnpress, .name = BTNPRESS },
    { .evfn = enternotify, .name = ENTERNOTIFY },
    { .evfn = propertynotify, .name = PROPERTYNOTIFY }
  };

  // Init return events
  for (size_t i = 0; i < LEN(EV); i++)
    init_event(&EV[i]);
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
