#include <X11/Xutil.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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

  const pair_t DPYSIZE = { DisplayWidth(dpy, SCRN), DisplayHeight(dpy, SCRN) };
  XUngrabKey(dpy, AnyKey, AnyModifier, ROOT);
  XModifierKeymap* modmap = XGetModifierMapping(dpy);
  unsigned numlockmask = { 0 };
  for (int k = 0; k < 8; k++)
    for (int j = 0; j < modmap->max_keypermod; j++)
      if (modmap->modifiermap[modmap->max_keypermod * k + j] == 
        XKeysymToKeycode(dpy, XK_Num_Lock))
        numlockmask = (1 << k);
  
  XFreeModifiermap(modmap);
  const int MODMASK = ~(numlockmask | LockMask);

  for (int i = 0; i < LEN(INPUT); i++) {
    const int MOD = INPUT[i].mod, KEY = INPUT[i].key;
    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, ROOT, 
      true, GrabModeAsync, GrabModeAsync);
  }
    
  const X_t X = { dpy, SCRN, ROOT, DPYSIZE, MODMASK };
  if (signal(SIGINT, sighandler) == SIG_ERR)
    sig_status = 1;

  init_clients(&X);
  init_panel(&X, BARH);
  draw_root(&X, WMNAME, strlen(WMNAME), TITLEFG, TITLEBG);
  events(&X, &sig_status);
  // Cleanup
  deinit_panel(&X);
  for (int i = 0; i < LEN(INPUT); i++) {
    const int MOD = INPUT[i].mod, KEY = INPUT[i].key;
    XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, ROOT);
  }

  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XCloseDisplay(dpy);
  return 0;
}
