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
static client_t* client;
static monitor_t* monitor;
static unsigned wks;
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
  // Blocks are deinit immediately upon failure so no checks here
  // Deinit client elements
  for (size_t n = 0; n < clients.size; n++) {
    client_t* client = (client_t*) clients.blk + n;
    deinit_gc(dpy, client->gc);
  }
  
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
  
  const pair_t POS = { clients.size ? client->pos.x + BARH : 0, 
    clients.size ? client->pos.y + BARH : BARH };
  const pair_t SIZE = { wa.width, wa.height };
  client_t client_ = { W, POS, SIZE, init_gc(dpy, ROOTW) };
  client_t* client_p = map_dev(&clients, &client_);
  if (client_p) {
    if (client)
      unfocus(dpy, ROOTW, client->w);

    focus(dpy, ROOTW, client_p->w);
    client = client_p;
    static const int WMASK = EnterWindowMask | 
      FocusChangeMask |
      PropertyChangeMask | 
      StructureNotifyMask;
    XSelectInput(dpy, W, WMASK);
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
      (unsigned char*) &W, 1);
    XMoveWindow(dpy, W, POS.x, POS.y);
    XMapWindow(dpy, W);
  }
}

void unmap(Display* dpy, const Window ROOTW, const Window W) {
  // Validate client
  if (!client || client->w != W)
    return;

  fprintf(stdout, "unmap(): window %ld\n", W);
  fprintf(stdout, "unmap(): window %ld\n", client->w);
  deinit_gc(dpy, client->gc);
  prev(dpy, ROOTW);
  unmap_dev(&clients, client);

  XDeleteProperty(dpy, ROOTW, atom.CLIENT_LIST);
  for (size_t n = 0; n < clients.size; n++)
    XChangeProperty(dpy, ROOTW, atom.CLIENT_LIST, XA_WINDOW, 32, PropModeAppend, 
      (const unsigned char*) ((client_t*) clients.blk + n)->w, 1);
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

static void unfocus(Display* dpy, const Window ROOTW, const Window W) {
  const int MODMASK = modmask(dpy);
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, W);
  }
  
  if (client) {
    XDeleteProperty(dpy, client->w, atom.ACTIVE_WINDOW);
    XSetWindowBorder(dpy, client->w, (size_t) INACTBDR);
  }
}

static void focus(Display* dpy, const Window ROOTW, const Window W) {
  // Activates
  XSetWindowBorder(dpy, W, (size_t) ACTBDR);
  XSetWindowBorderWidth(dpy, W, BDRW);
  XSetInputFocus(dpy, W, RevertToPointerRoot, CurrentTime);
  XChangeProperty(dpy, ROOTW, atom.WM_STATE, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XChangeProperty(dpy, W, atom.ACTIVE_WINDOW, XA_WINDOW, 32, PropModeReplace, 
    (const unsigned char*) &W, 1);
  XRaiseWindow(dpy, W);
  static const int BUTTONMASK = ButtonPressMask | ButtonReleaseMask;
  const int MODMASK = modmask(dpy);
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, W);
    XGrabButton(dpy, KEY, MOD & MODMASK, W, false, BUTTONMASK, GrabModeSync, 
      GrabModeSync, None, None);
  }
}

static client_t* find_client(const client_t* client_p) {
  /*
  size_t n = 0;
  for (; n < clients.size + 1 && ((client_t*) clients.blk + n)->w != W; n++);
  return n < clients.size ? (client_t*) clients.blk + n : NULL;
  */
  return find_dev(&clients, client_p);
}
////////////////////////////////////////////////////////////////////////////////
// Commands                                                                   //
////////////////////////////////////////////////////////////////////////////////
static void quit(Display*, const Window) {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Kill\n");
  if (client) {
    XEvent xev = { ClientMessage };
    xev.xclient.window = client->w;
    xev.xclient.message_type = atom.PROTO;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = atom.DELETE_WINDOW;
    xev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, client->w, false, NoEventMask, &xev);
  }
}

static void prev(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Prev\n");
  /*
  if (clients.size > 1) {
    client_t* client_p = client == (client_t*) clients.blk ? 
      (client_t*) clients.blk + clients.size - 1 : client - 1;
  }
  */
  client_t* client_p = prev_dev(&clients, client);
  unfocus(dpy, ROOTW, client->w);
  focus(dpy, ROOTW, client_p->w);
  client = client_p;
}

static void next(Display* dpy, const Window ROOTW) {
  fprintf(stdout, "Next\n");
  /*
  if (clients.size > 1) {
    client_t* client_p = client == (client_t*) clients.blk + clients.size - 1 ? 
      (client_t*) clients.blk : client + 1;
    focus(dpy, ROOTW, client_p);
  }
  */

  client_t* client_p = next_dev(&clients, client);
  unfocus(dpy, ROOTW, client->w);
  focus(dpy, ROOTW, client_p->w);
  client = client_p;
}
