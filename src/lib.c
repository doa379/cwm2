#include <X11/Xutil.h>
#include <lib.h>

atom_t init_atoms(Display* dpy) {
  return (atom_t) {
    XInternAtom(dpy, "WM_PROTOCOLS", false),
    XInternAtom(dpy, "WM_NAME", false),
    XInternAtom(dpy, "WM_DELETE_WINDOW", false),
    XInternAtom(dpy, "WM_STATE", false),
    XInternAtom(dpy, "WM_TAKE_FOCUS", false),
    XInternAtom(dpy, "_NET_SUPPORTED", false),
    XInternAtom(dpy, "_NET_WM_STATE", false),
    XInternAtom(dpy, "_NET_WM_NAME", false),
    XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false),
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false),
    XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false),
    XInternAtom(dpy, "_NET_CLIENT_LIST", false),
    XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false),
    XInternAtom(dpy, "_NET_WM_DESKTOP", false),
    XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false),
    XInternAtom(dpy, "_NET_SHOWING_DESKTOP", false)
  };
}

int modmask(Display* dpy) {
  XModifierKeymap* modmap = XGetModifierMapping(dpy);
  unsigned numlockmask = { 0 };
  for (int k = 0; k < 8; k++)
    for (int j = 0; j < modmap->max_keypermod; j++)
      if (modmap->modifiermap[modmap->max_keypermod * k + j] == 
        XKeysymToKeycode(dpy, XK_Num_Lock))
        numlockmask = (1 << k);
  
  XFreeModifiermap(modmap);
  return ~(numlockmask | LockMask);
}
