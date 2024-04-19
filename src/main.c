#include <stdio.h>
#include <signal.h>
#include <Xlib.h>
#include <wm.h>
#include <lib.h>
#include <draw.h>
#include <dbus.h>

volatile sig_atomic_t sig_status;

static void sighandler(int sig) {
  sig_status = 1;
  fprintf(stdout, "\nSig.\n");
}

int main(const int ARGC, const char* ARGV[]) {
  if (signal(SIGINT, sighandler) == SIG_ERR) {
    fprintf(stderr, "Critical: Failed to set handler\n");
    return -1;
  }

  Display* dpy = { XOpenDisplay(NULL) };
  if (!dpy) {
    fprintf(stderr, "Critical: Failed to open display\n");
    return -1;
  } 

  if (!init_root(dpy)) {
    XCloseDisplay(dpy);
    fprintf(stderr, "Critical: Initialization error (another wm running?)\n");
    return -1;
  }

  if (!init_wks()) {
    XCloseDisplay(dpy);
    fprintf(stderr, 
      "Critical: Initialization error (Failed to alloc. workspaces)\n");
    return -1;
  }
  
  if (!init_monitors()) {
    deinit_wks();
    XCloseDisplay(dpy);
    fprintf(stderr, 
      "Critical: Initialization error (Failed to alloc. monitors)\n");
    return -1;
  }

  init_atoms(dpy);
  init_draw(dpy);
  // Init internal events
  init_events();
  //
  static ev_t EV[] = {
    { .evfn = noop, .EVENT = NOOP },
    { .evfn = mapnotify, .EVENT = MAPNOTIFY },
    { .evfn = unmapnotify, .EVENT = UNMAPNOTIFY },
    //{ .evfn = clientmessage, .EVENT = CLIENTMESSAGE },
    { .evfn = configureroot, .EVENT = CONFIGUREROOT },
    { .evfn = configurenotify, .EVENT = CONFIGURENOTIFY },
    { .evfn = maprequest, .EVENT = MAPREQUEST },
    { .evfn = noop, .EVENT = CONFIGUREREQUEST },
    { .evfn = motionnotify, .EVENT = MOTIONNOTIFY },
    { .evfn = keypress, .EVENT = KEYPRESS },
    { .evfn = btnpress, .EVENT = BTNPRESS },
    { .evfn = enternotify, .EVENT = ENTERNOTIFY },
    { .evfn = propertynotify, .EVENT = PROPERTYNOTIFY },
    { .evfn = exposeroot, .EVENT = EXPOSE },
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
  {
    const size_t N = { sizeof EV / sizeof EV[0] };
    for (size_t i = { 0 }; i < N; i++)
      init_event(&EV[i]);
  }

  {
    const size_t N = { sizeof MSGEV / sizeof MSGEV[0] };
    for (size_t i = { 0 }; i < N; i++)
      init_msgevent(&MSGEV[i]);
  }

  if (!init_dbus())
    fprintf(stderr,
      "Warning: Initialization with DBus error (Won't provide DBus comms.)\n");

  init_wm();
  dbus_send("Status", "cwm2 initialized", CRITICAL, 1500);
  while (sig_status == 0) {
    const ev_t* EV = { event(dpy) };
    EV->evfn(EV->DATA[0], EV->DATA[1], EV->DATA[2]);
  }

  // Cleanup
  dbus_send("Status", "cwm2 exit", CRITICAL, 1500);
  deinit_wm();
  deinit_dbus();
  deinit_draw();
  deinit_monitors();
  deinit_wks();
  deinit_root();
  XCloseDisplay(dpy);
  return 0;
}
