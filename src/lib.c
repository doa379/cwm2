#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <lib.h>

pair_t init_window(Display* dpy, const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return (pair_t) { 0 };
  
  static const int WMASK = EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask;
  XSelectInput(dpy, W, WMASK);
  return (pair_t) { wa.width, wa.height };
}

void append_window(Display* dpy, const Window root, const Window W, const Atom PROP) {
  XChangeProperty(dpy, root, PROP, XA_WINDOW, 32, PropModeAppend,
    (unsigned char*) &W, 1);
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
