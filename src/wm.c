#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdbool.h>
#include <util.h>
#include <../config.h>

typedef struct {
  Display* dpy;
  int scrn;
  Window root;
  pair_t dpysize;
  int modmask;
} X_t;

static bool xerror;
static const int ROOTMASK = { 
  SubstructureRedirectMask | 
  SubstructureNotifyMask | 
  ButtonPressMask |
  PointerMotionMask |
  EnterWindowMask |
  LeaveWindowMask |
  StructureNotifyMask |
  PropertyChangeMask
};

static void events(const X_t* X) {

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
  events(&X);
  // Cleanup
  for (int i = 0; i < LEN(INPUT); i++) {
    const int MOD = INPUT[i].mod, KEY = INPUT[i].key;
    XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, ROOT);
  }

  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XCloseDisplay(dpy);
  return 0;
}
