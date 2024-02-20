#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wm.h>
#include <lib.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <../config.h>

typedef struct {
  void* clients;
  size_t reserve;
  size_t size;
  client_t* prev;
  client_t* curr;
} C_t;

static Display* dpy;
static Window rootw;
static atom_t atom;
static C_t C;
static void (*CALLFN[])() = {
  [QUIT] = quit,
  [KILL] = kill
};

static client_t* init_client(const client_t* client) {
  // Return address of client
  if (C.size < C.reserve) {
    memcpy((client_t*) C.clients + C.size, client, sizeof(client_t));
    C.size++;
    return (client_t*) C.clients + C.size - 1;
  } else {
    void* clients = realloc(C.clients, (C.size + C.reserve) * sizeof(client_t));
    if (clients) {
      C.clients = clients;
      memcpy((client_t*) C.clients + C.size, client, sizeof(client_t));
      C.reserve += C.size;
      C.size++;
      return (client_t*) C.clients + C.size - 1;
    }
  }

  return NULL;
}

static void deinit_client(client_t* client) {
  // Patt: move post segment
  int n = 0;
  for (; n < C.size && ((client_t*) C.clients + n)->w != client->w; n++);
  memcpy((client_t*) C.clients + n, (client_t*) C.clients + n + 1,
    (C.size - n - 1) * sizeof(client_t));
  C.size--;
}

static bool init_clients() {
  static const int MAXNCLI = 2;
  if ((C.clients = calloc(MAXNCLI, sizeof(client_t)))) {
    C.reserve = MAXNCLI;
    C.size = 0;
    C.prev = C.curr = C.clients;
    return true;
  }

  return false;
}

static void deinit_clients() {
  free(C.clients);
  C.reserve = C.size = 0;
  C.prev = C.curr = NULL;
}

bool init_wm(Display* dpy_, const Window W) {
  if (init_clients()) {
    dpy = dpy_;
    rootw = W;
    atom = init_atoms(dpy);
    init_windows();
    return true;
  }

  return false;
}

void deinit_wm() {
  deinit_clients();
}

void key(const int STATE, const int CODE) {
  const int KMOD = STATE & modmask(dpy);
  const int KSYM = XkbKeycodeToKeysym(dpy, CODE, 0, 0);
  for (size_t i = 0; i < LEN(KBD); i++)
    if (KBD[i].mod == KMOD && KBD[i].key == KSYM) {
      if (KBD[i].call < 128) {
        CALLFN[KBD[i].call]();
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

void map(const Window W) {
  static XWindowAttributes wa;
  if (XGetWindowAttributes(dpy, W, &wa) == 0 || wa.override_redirect)
    return;
  
  static const int WMASK = EnterWindowMask | 
    FocusChangeMask |
    PropertyChangeMask | 
    StructureNotifyMask;
  XSelectInput(dpy, W, WMASK);
  XMapWindow(dpy, W);
  XChangeProperty(dpy, rootw, atom.CLIENT_LIST, XA_WINDOW, 32, 
    PropModeAppend, (unsigned char*) &W, 1);
  const pair_t POS = { C.size ? C.curr->pos.x + BARH : 0, 
    C.size ? C.curr->pos.y + BARH : BARH };
  const pair_t SIZE = { wa.width, wa.height };
  client_t client = { W, POS, SIZE };
  client_t* curr = init_client(&client);
  if (curr) {
    focus(curr);
    XMoveWindow(dpy, W, POS.x, POS.y);
  }
}

static void init_windows() {
  Window root;
  Window par;
  Window* w;
  unsigned n;
  if (XQueryTree(dpy, rootw, &root, &par, &w, &n)) {
    for (int i = 0; i < n; i++) {
      XWindowAttributes wa;
      if (XGetWindowAttributes(dpy, w[i], &wa) && wa.map_state == IsViewable) {
        XEvent xev = { MapRequest };
        xev.xmaprequest.send_event = true,
        xev.xmaprequest.parent = rootw;
        xev.xmaprequest.window = w[i];
        XSendEvent(dpy, rootw, true, ROOTMASK, &xev);
      }
    }

    if (w)
      XFree(w);
  }

  XSync(dpy, false);
}

static void focus(client_t* client) {
  const int MODMASK = modmask(dpy);
  // Deactivates
  for (size_t i = 0; i < LEN(BTN); i++) {
    const int MOD = BTN[i].mod;
    const int KEY = BTN[i].key;
    XUngrabButton(dpy, KEY, MOD & MODMASK, rootw);
  }
  
  XDeleteProperty(dpy, C.curr->w, atom.ACTIVE_WINDOW);
  XSetWindowBorder(dpy, C.curr->w, (size_t) INACTBDR);
  C.prev = C.curr;
  // Activates
  XSetWindowBorder(dpy, client->w, (size_t) ACTBDR);
  XSetWindowBorderWidth(dpy, client->w, BDRW);
  XSetInputFocus(dpy, client->w, RevertToPointerRoot, CurrentTime);
  XChangeProperty(dpy, rootw, atom.WM_STATE, XA_WINDOW, 32, 
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

  C.curr = client;
}

static void quit() {
  fprintf(stdout, "Quit\n");
  raise(SIGINT);
}

static void kill() {
  fprintf(stdout, "Kill\n");
}
