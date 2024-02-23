#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <panel.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <../config.h>

static blk_t clients;
static blk_t monitors;
static client_t* prev_client;
static client_t* curr_client;
static monitor_t* monitor;
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

void configure_root(Display* dpy, const Window ROOTW, const int WIDTH, const int HEIGHT) {

}

void map(Display* dpy, const Window ROOTW, const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return;
  
  const pair_t POS = { clients.size ? curr_client->pos.x + BARH : 0, 
    clients.size ? curr_client->pos.y + BARH : BARH };
  const pair_t SIZE = { wa.width, wa.height };
  client_t client = { W, POS, SIZE };
  const Window W0 = prev_client ? prev_client->w : 0;
  const Window W1 = curr_client ? curr_client->w : 0;
  // May alter ptrs prev, curr
  client_t* client_p = init_dev(&clients, &client);
  if (client_p) {
    prev_client = W0 ? find_client(W0) : NULL;
    curr_client = W1 ? find_client(W1) : NULL;
    focus(dpy, ROOTW, client_p);
    static const int WMASK = EnterWindowMask | 
      FocusChangeMask |
      PropertyChangeMask | 
      StructureNotifyMask;
    XSelectInput(dpy, W, WMASK);
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, 
      PropModeAppend, (unsigned char*) &W, 1);
    XMoveWindow(dpy, W, POS.x, POS.y);
    XMapWindow(dpy, W);
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

static void focus(Display* dpy, const Window ROOTW, client_t* client) {
  const int MODMASK = modmask(dpy);
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, ROOTW);
  }
  
  if (curr_client) {
    XDeleteProperty(dpy, curr_client->w, atom.ACTIVE_WINDOW);
    XSetWindowBorder(dpy, curr_client->w, (size_t) INACTBDR);
    prev_client = curr_client;
  }

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

  curr_client = client;
}

static void quit(Display*, const Window) {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Kill\n");
}

static client_t* find_client(const Window W) {
  int n = 0;
  for (; n < clients.size && ((client_t*) clients.blk + n)->w != W; n++);
  return (client_t*) clients.blk + n;
}

static void prev(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Prev\n");
  if (clients.size > 1) {
    client_t* client = curr_client == (client_t*) clients.blk ? 
      (client_t*) clients.blk + clients.size - 1 : curr_client - 1;
    focus(dpy, ROOTW, client);
  }
}

static void next(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Next\n");
  if (clients.size > 1) {
    client_t* client = curr_client == (client_t*) clients.blk + clients.size - 1 ? 
      (client_t*) clients.blk : curr_client + 1;
    focus(dpy, ROOTW, client);
  }
}
