#include <X11/Xutil.h>
#include <lib.h>

bool init_window(Display* dpy, const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return false;
  
  static const int WMASK = EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask;
  XSelectInput(dpy, W, WMASK);
  //XChangeProperty(X->dpy, X->root, root.atom.CLIENT_LIST, XA_WINDOW, 32, 
      //PropModeAppend, (unsigned char*) &W, 1);

  return true;
}

/*
  proc: position --> map --> focus

  if (T.size()) {
    const auto POS { arrange(wa.width, wa.height) };
    wa.x = std::get<0>(POS);
    wa.y = std::get<1>(POS);
  }

  wa.y = wa.y < BARH ? BARH : wa.y;
  XMoveWindow(root.dpy, W, wa.x, wa.y);
  XMapWindow(root.dpy, W);
  
  prev = curr;
  if (T.size())
    set_inact(curr->w);
  T.emplace_back(Client { 
    W, p.new_gc(), { wa.x, wa.y }, { wa.width, wa.height } });
  curr = T.cend() - 1;
  set_act(curr->w);
*/
  
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
