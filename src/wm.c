#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <../config.h>

static blk_t clients;
static blk_t monitors;
static atom_t atom;
static void (*CALLFN[])(Display*, const Window) = {
  [QUIT] = quit,
  [KILL] = kill,
  [PREVCLI] = prev,
  [NEXTCLI] = next
};

bool init_wm(Display* dpy, const Window W) {
  // Initial reserve is working correctly
  clients = init_blk(sizeof(client_t), 2);
  if (clients.blk == NULL) 
    return false;

  // Initial reserve is working correctly
  monitors = init_blk(sizeof(monitor_t), 2);
  if (monitors.blk == NULL) {
    deinit_blk(&clients);
    return false;
  }

  XUngrabKey(dpy, AnyKey, AnyModifier, W);
  const int MODMASK = modmask(dpy);
  for (size_t i = 0; i < LEN(KBD); i++) {
    const int MOD = KBD[i].mod;
    const int KEY = KBD[i].key;
    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, W, 
      true, GrabModeAsync, GrabModeAsync);
  }

  atom = init_atoms(dpy);
  init_windows(dpy, W);
  return true;
}

void deinit_wm(Display* dpy, const Window W) {
  // Blocks are deinit. immediately upon failure so no checks here
  deinit_blk(&clients);
  deinit_blk(&monitors);
  const int MODMASK = modmask(dpy);
  for (size_t i = 0; i < LEN(KBD); i++) {
    const int MOD = KBD[i].mod;
    const int KEY = KBD[i].key;
    XUngrabKey(dpy, XKeysymToKeycode(dpy, KEY), MOD & MODMASK, W);
  }
}

void key(Display* dpy, const Window W, const int STATE, const int CODE) {
  const int KMOD = STATE & modmask(dpy);
  const int KSYM = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  for (size_t i = 0; i < LEN(KBD); i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call](dpy, W);
      } else if (KBD[i].cmd) {
        fprintf(stdout, "spawn %s\n", KBD[i].cmd);
        if (fork() == 0) {
          close(ConnectionNumber(dpy));
          setsid();
          system(KBD[i].cmd);
        }
      }
    }
}

void map(Display* dpy, const Window ROOTW, const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return;
  
  const pair_t POS = { clients.size ? ((client_t*) clients.curr)->pos.x + BARH : 0, 
    clients.size ? ((client_t*) clients.curr)->pos.y + BARH : BARH };
  const pair_t SIZE = { wa.width, wa.height };
  client_t client = { W, POS, SIZE };
  client_t* curr = init_dev(&clients, &client);
  if (curr) {
    static const int WMASK = EnterWindowMask | 
      FocusChangeMask |
      PropertyChangeMask | 
      StructureNotifyMask;
    XSelectInput(dpy, W, WMASK);
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, 
    PropModeAppend, (unsigned char*) &W, 1);
    XMoveWindow(dpy, W, POS.x, POS.y);
    XMapWindow(dpy, W);
    focus(dpy, ROOTW, curr);
  }
}

static void focus(Display* dpy, const Window ROOTW, client_t* client) {
  const int MODMASK = modmask(dpy);
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, ROOTW);
  }
  
  XDeleteProperty(dpy, ((client_t*) clients.curr)->w, atom.ACTIVE_WINDOW);
  XSetWindowBorder(dpy, ((client_t*) clients.curr)->w, (size_t) INACTBDR);
  clients.prev = clients.curr;
  // Activates
  XSetWindowBorder(dpy, client->w, (size_t) ACTBDR);
  XSetWindowBorderWidth(dpy, client->w, BDRW);
  XSetInputFocus(dpy, client->w, RevertToPointerRoot, CurrentTime);
  XChangeProperty(dpy, ROOTW, atom.WM_STATE, XA_WINDOW, 32, 
    PropModeReplace, (const unsigned char*) &client->w, 1);
  XChangeProperty(dpy, client->w, atom.ACTIVE_WINDOW, XA_WINDOW, 32, 
    PropModeReplace, (const unsigned char*) &client->w, 1);
  XRaiseWindow(dpy, client->w);
  static const int BUTTONMASK = ButtonPressMask | ButtonReleaseMask;
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, client->w);
    XGrabButton(dpy, KEY, MOD & MODMASK, client->w, false, BUTTONMASK, GrabModeSync, 
      GrabModeSync, None, None);
  }

  clients.curr = client;
}

static void quit(Display*, const Window) {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Kill\n");
}

static void prev(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Prev\n");
  if (clients.size > 1) {
    client_t* client = clients.curr == (client_t*) clients.blk ? 
      (client_t*) clients.blk + clients.size - 1 : (client_t*) clients.curr - 1;
    focus(dpy, ROOTW, client);
  }
}

static void next(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Next\n");
  if (clients.size > 1) {
    client_t* client = clients.curr == (client_t*) clients.blk + clients.size - 1 ? 
      (client_t*) clients.blk : (client_t*) clients.curr + 1;
    focus(dpy, ROOTW, client);
  }
}
