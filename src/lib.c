#include <stdlib.h>
#include <string.h>
#include <X11/Xutil.h>
#include <lib.h>

blk_t init_blk(const size_t UNITSIZE, const size_t RESERVE) {
  void* blk = calloc(RESERVE, UNITSIZE);
  return (blk_t) { blk, UNITSIZE, RESERVE, 0 };
}

void deinit_blk(blk_t* blk) {
  free(blk->blk);
  *blk = (blk_t) { 0 };
}

void* init_dev(blk_t* blk, const void* dev) {
  // Return address of dev
  if (blk->size < blk->reserve) {
    memcpy((char*) blk->blk + blk->size * blk->unit, dev, blk->unit);
    blk->size++;
    return (char*) blk->blk + (blk->size - 1) * blk->unit;
  } else {
    void* ptr = realloc(blk->blk, (blk->size + blk->reserve) * blk->unit);
    if (ptr) {
      blk->blk = ptr;
      blk->reserve += blk->size;
      memcpy((char*) blk->blk + blk->size * blk->unit, dev, blk->unit);
      blk->size++;
      return (char*) blk->blk + (blk->size - 1) * blk->unit;
    }
  }

  return NULL;
}

void deinit_dev(blk_t* blk, const void* dev) {
  // Patt: move post segment
  int n = 0;
  for (; n < blk->size && (char*) blk->blk + n * blk->unit != dev; n++);
  memcpy((char*) blk->blk + n * blk->unit, (char*) blk->blk + (n + 1) * blk->unit,
    (blk->size - n - 1) * blk->unit);
  blk->size--;
}

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

void init_windows(Display* dpy, const Window W) {
  Window root;
  Window par;
  Window* w;
  unsigned n;
  if (XQueryTree(dpy, W, &root, &par, &w, &n)) {
    for (unsigned i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = W;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, W, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
}
