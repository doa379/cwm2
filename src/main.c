#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <wm.h>
#include <lib.h>
#include <events.h>
#include <panel.h>
#include <../config.h>

static const char* WMNAME = "cwm2";
static const char* WMVER = "-0.0";
volatile sig_atomic_t sig_status;
static bool xerror;

static void sighandler(int sig) {
  sig_status = 1;
  fprintf(stdout, "\n%s exit\n", WMNAME);
}

static int XError(Display*, XErrorEvent* xev) {
  xerror = xev->error_code == BadAccess;
  return 0;
}

int main(const int ARGC, const char* ARGV[]) {
  Display* dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "Failed to open display\n");
    return -1;
  }
  
  const int SCRN = DefaultScreen(dpy);
  const Window ROOT = XRootWindow(dpy, SCRN);
  XSetErrorHandler(XError);
  XSelectInput(dpy, ROOT, ROOTMASK);
  if (xerror) {
    XCloseDisplay(dpy);
    fprintf(stderr, "Initialization error (another wm running?)");
    return -1;
  }

  if (!init_wm(dpy, ROOT)) {
    XCloseDisplay(dpy);
    fprintf(stderr, "Initialization error (Failed to alloc for clients)");
    return -1;
  }

  init_panel(dpy, SCRN, ROOT, BARH);
  draw_root(dpy, ROOT, WMNAME, strlen(WMNAME), TITLEFG, TITLEBG);

  XUngrabKey(dpy, AnyKey, AnyModifier, ROOT);
  {
    const int MODMASK = modmask(dpy);
    for (int i = 0; i < LEN(KBD); i++) {
      const int MOD = KBD[i].mod;
      const int KEY = KBD[i].key;
      XGrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, ROOT, 
        true, GrabModeAsync, GrabModeAsync);
    }
  }

  if (signal(SIGINT, sighandler) == SIG_ERR)
    sig_status = 1;
  
  events(dpy, ROOT, &sig_status);
  // Cleanup
  deinit_panel(dpy);
  deinit_wm();
  {
    const int MODMASK = modmask(dpy);
    for (int i = 0; i < LEN(KBD); i++) {
      const int MOD = KBD[i].mod;
      const int KEY = KBD[i].key;
      XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, ROOT);
    }
  }

  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XCloseDisplay(dpy);
  return 0;
}
